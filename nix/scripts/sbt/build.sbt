name := "ledger-lib-core"

organization := "co.ledger"

version := sys.env.getOrElse("JAR_VERSION", "")

scalaVersion := "2.12.8"

ThisBuild / githubOwner := "LedgerHQ"
ThisBuild / githubRepository := "lib-ledger-core"
ThisBuild / githubTokenSource := TokenSource.Environment("GITHUB_TOKEN") || TokenSource.Environment("GH_TOKEN") || TokenSource.GitConfig("github.token")
ThisBuild / versionScheme := Some("strict")
