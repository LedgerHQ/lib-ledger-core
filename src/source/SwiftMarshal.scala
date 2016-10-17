package djinni

import djinni.ast.{Interface, TypeDef, TypeRef}
import djinni.generatorTools.Spec
import djinni.meta._

/**
  *
  * SwiftMarshall
  * src
  *
  * Created by Pierre Pollastri on 03/10/2016.
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
class SwiftMarshal(spec: Spec) extends Marshal(spec) {
  // Typename string to be used to declare a type or template parameter, without namespace or package, except for extern types which are always fully qualified.
  override def typename(tm: MExpr): String = ???

  def typename(name: String, ty: TypeDef): String = idSwift.ty(name)

  def objcTypename(name: String, ty: TypeDef): String = idObjc.ty(name)

  override def returnType(ret: Option[TypeRef]): String = ret.map {x =>
    toSwiftType(x.resolved, false)._1
  }.getOrElse("Void")

  override def fqParamType(tm: MExpr): String = toSwiftType(tm, false)._1

  // Same as typename() but always fully namespace or package qualified
  override def fqTypename(tm: MExpr): String = ???

  override def fqFieldType(tm: MExpr): String = ???

  override def fqReturnType(ret: Option[TypeRef]): String = ???

  // Type signature for a function parameter
  override def paramType(tm: MExpr): String = toSwiftType(tm, false)._1

  override def fieldType(tm: MExpr): String = ???

  private def toSwiftType(tm: MExpr, needRef: Boolean): (String, Boolean) = {
    def args(tm: MExpr) = if (tm.args.isEmpty) "" else ""//tm.args.map(toBoxedParamType).mkString("<", ", ", ">")
    def f(tm: MExpr, needRef: Boolean): (String, Boolean) = {
      tm.base match {
        case MOptional =>
          // We use "nil" for the empty optional.
          assert(tm.args.size == 1)
          val arg = tm.args.head
          arg.base match {
            case MOptional => throw new AssertionError("nested optional?")
            case m => f(arg, true) match {
              case (x, y) =>
                (x + "?", y)
            }
          }
        case o =>
          val base = o match {
            case p: MPrimitive => if (needRef) (p.objcBoxed, true) else (p.swiftName, false)
            case MString => ("String", true)
            case MDate => ("NSDate", true)
            case MBinary => ("NSData", true)
            case MOptional => throw new AssertionError("optional should have been special cased")
            case MList => ("NSArray" + args(tm), true)
            case MSet => ("NSSet" + args(tm), true)
            case MMap => ("NSDictionary" + args(tm), true)
            case d: MDef => d.defType match {
              case DEnum => if (needRef) ("NSNumber", true) else (idObjc.ty(d.name), false)
              case DRecord => (idObjc.ty(d.name), true)
              case DInterface =>
                val ext = d.body.asInstanceOf[Interface].ext
                if (!ext.objc)
                  (idObjc.ty(d.name), true)
                else
                  (s"id<${idObjc.ty(d.name)}>", false)
            }
            case e: MExtern => e.body match {
              case i: Interface => if(i.ext.objc) (s"id<${e.objc.typename}>", false) else (e.objc.typename, true)
              case _ => if(needRef) (e.objc.boxed, true) else (e.objc.typename, e.objc.pointer)
            }
            case p: MParam => throw new AssertionError("Parameter should not happen at Obj-C top level")
          }
          base
      }
    }
    f(tm, needRef)
  }
}
