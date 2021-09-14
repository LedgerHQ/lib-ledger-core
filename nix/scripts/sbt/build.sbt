name := "ledger-lib-core"

organization := "co.ledger"

version := sys.env.getOrElse("JAR_VERSION", "")

scalaVersion := "2.12.8"

githubOwner := "LedgerHQ"
githubRepository := "lib-ledger-core"

lazy val core = project.settings(
    name := "ledger-lib-core",
)

githubTokenSource := TokenSource.Environment("GITHUB_TOKEN")
