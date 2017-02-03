package djinni
import djinni.ast._
import djinni.meta.Meta

import scala.collection.immutable
import scala.collection.mutable.ArrayBuffer
import scala.util.{Success, Try}

/**
  *
  * preprocessor
  * src
  *
  * Created by Pierre Pollastri on 02/02/2017.
  *
  * The MIT License (MIT)
  *
  * Copyright (c) 2016 Ledger
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  *
  */


object preprocessor {

  type Scope = immutable.Map[String,Meta]

  private def isGenericType(decl: TypeDecl) = {
    decl.body match {
      case interface: Interface => interface.generic.nonEmpty
      case other => false
    }
  }

  private def resolveTypeExpr(expr: TypeExpr, resolution: Map[String, String]): TypeExpr = {
    val ident = Ident(
      resolution.toSeq.filter({(pair) =>
        pair._1 == expr.ident.name
      }).map(_._2).headOption.getOrElse(expr.ident.name),
      expr.ident.file,
      expr.ident.loc
    )
    TypeExpr(ident, expr.args.map(resolveTypeExpr(_, resolution)))
  }

  private def resolveTypeRef(ref: TypeRef, resolution: Map[String, String]): TypeRef = {
    TypeRef(resolveTypeExpr(ref.expr, resolution))
  }

  private def createInterfaceIdent(generic: TypeDecl, resolution: Map[String, String]): Ident = {
    val interface = generic.body.asInstanceOf[Interface]
    val name = s"${interface.generic.map({(t) => resolution(t.placeholder).capitalize}).mkString("")}${generic.ident.name}"
    Ident(name, generic.ident.file, generic.ident.loc)
  }

  private def createTypeFromGeneric(typeExpr: TypeExpr, generic: TypeDecl): TypeDecl = {
    val genericInterface = generic.body.asInstanceOf[Interface]
    val genericResolution = (genericInterface.generic.map(_.placeholder) zip typeExpr.args.map(_.ident.name)).toMap
    println(s"Specializing ${generic.ident.name} with ${genericResolution}")
    val methods = genericInterface.methods map {(genericMethod) =>
      val ret = genericMethod.ret map {(returnType) =>
        resolveTypeRef(returnType, genericResolution)
      }
      val params = genericMethod.params map {(field) =>
        Field(field.ident, resolveTypeRef(field.ty, genericResolution), field.doc)
      }
      Interface.Method(genericMethod.ident, params, ret, genericMethod.doc, genericMethod.static, genericMethod.const)
    }
    val interface: Interface = Interface(genericInterface.ext, methods, genericInterface.consts, Seq())
    generic match {
      case intern: InternTypeDecl =>
        InternTypeDecl(createInterfaceIdent(intern, genericResolution), Seq(), interface, intern.doc, intern.origin)
      case extern: ExternTypeDecl =>
        ExternTypeDecl(createInterfaceIdent(extern, genericResolution), Seq(), interface, extern.properties, extern.origin)
    }
  }

  private def createTypeRefFromGenericRef(ref: TypeRef): TypeRef = {
    val name = s"${ref.expr.args.map({(t) => t.ident.name.capitalize}).mkString("")}${ref.expr.ident.name}"
    println(s"Create type $name")
    val ident = Ident(name, ref.expr.ident.file, ref.expr.ident.loc)
    TypeRef(TypeExpr(ident, Seq()))
  }

  private def resolveGenericTypeFromTypeExpr(typeExpr: TypeExpr, generics: Seq[TypeDecl]): Option[TypeDecl] = {
    generics find {(decl) =>
      decl.body match {
        case interface: Interface =>
          if (decl.ident.name == typeExpr.ident.name)
            true
          else
            false
        case other => false
      }
    }
  }

  private def resolveTypeRef(typeRef: TypeRef, generics: Seq[TypeDecl], specializedGenerics: ArrayBuffer[TypeDecl]): TypeRef = {
    if (typeRef.expr.args.nonEmpty) {
      resolveGenericTypeFromTypeExpr(typeRef.expr, generics).map({(generic: TypeDecl) =>
        specializedGenerics.append(createTypeFromGeneric(typeRef.expr, generic))
        createTypeRefFromGenericRef(typeRef)
      }).getOrElse(typeRef)
    } else {
      typeRef
    }
  }

  private def resolveTypeDecl(declaration: TypeDecl, generics: Seq[TypeDecl]): Seq[TypeDecl] = {
    declaration.body match {
      case interface: Interface =>
        val specializedGenerics = ArrayBuffer[TypeDecl]()
        val methods = interface.methods map {(method) =>
          val ret = method.ret.map({(td) => resolveTypeRef(td, generics, specializedGenerics)})
          val params = method.params map {(field) =>
            Field(field.ident, resolveTypeRef(field.ty, generics, specializedGenerics), field.doc)
          }
          Interface.Method(method.ident, params, ret, method.doc, method.static, method.const)
        }
        val int = Interface(interface.ext, methods, interface.consts, interface.generic)
        val decl = declaration match {
          case internal: InternTypeDecl =>
            InternTypeDecl(declaration.ident, declaration.params, int, internal.doc, declaration.origin)
          case external: ExternTypeDecl =>
            ExternTypeDecl(declaration.ident, declaration.params, int, external.properties, declaration.origin)
        }
        Seq(decl) ++ specializedGenerics
      case others =>
        Seq(declaration)
    }
  }

  def resolveTemplates(metas: Scope, idl: Seq[TypeDecl]): Try[(Scope, Seq[TypeDecl])] = {
    val generics = idl.filter(isGenericType)

    generics foreach {(decl) =>
      System.out.println(s"Got template type ${decl.ident.name}")
    }

    val finalIdl = idl.filterNot(isGenericType).flatMap({(decl: TypeDecl) =>
      resolveTypeDecl(decl, generics)
    }).distinct

    Success(metas, finalIdl)
  }

  def printIdl(idl: Seq[TypeDecl]): Unit = {
    idl foreach {(i) =>
      i.body match {
        case interface: Interface =>
          println(i.ident.name + " {")
          interface.methods foreach {(m) =>
            def formatType(expr: TypeExpr): String = {
              if (expr.args.isEmpty)
                expr.ident.name
              else {
                val out = expr.ident.name
                out + s"<${expr.args.map(formatType).mkString(", ")}>"
              }
            }
            val params = m.params map {(p) =>
              formatType(p.ty.expr) + " " + p.ident.name
            }
            println(s" ${m.ret.map(_.expr.ident.name).getOrElse("void")} ${m.ident.name}(${params.mkString(", ")});")
          }
          println("}")
        case other => // Ignore
      }
    }
  }

}
