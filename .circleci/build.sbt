name := "scala-lib-core"

organization := "co.ledger"

version := ""

scalaVersion := "2.12.8"

lazy val core = project.settings(
    name := "scala-lib-core",
)

lazy val host = "archiva.prod.aws.ledger.fr"

credentials ++= {
    val repository = if (isSnapshot.value) "snapshots" else "internal"
    lazy val username = sys.env.get("ARCHIVA_USERNAME")
    lazy val password = sys.env.get("ARCHIVA_PASSWORD")
    (username, password) match {
        case (Some(u), Some(p)) =>
            Seq(Credentials(s"Repository Archiva Managed $repository Repository", host, u, p))
        case _ =>
            println("Missing Archiva credentials for publishing.")
            Seq.empty
    }
}

ThisBuild / publishTo := {
    val repository = if (isSnapshot.value) "snapshots" else "internal"
    Some(s"Ledger ${repository.capitalize}" at s"https://$host/repository/$repository")
}
