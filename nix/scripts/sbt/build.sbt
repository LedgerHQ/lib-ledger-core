name := "ledger-lib-core"

organization := "co.ledger"

version := sys.env.getOrElse("JAR_VERSION", "")

scalaVersion := "2.12.8"

ThisBuild / githubOwner := "LedgerHQ"
ThisBuild / githubRepository := "lib-ledger-core"
ThisBuild / githubTokenSource := TokenSource.Environment("GITHUB_TOKEN")
ThisBuild / versionScheme := Some("strict")

lazy val core = project.settings(
    name := "ledger-lib-core",
)

