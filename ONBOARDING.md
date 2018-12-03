# Ledger Lib Core Onboarding 

## First steps

1. Read the [README] document to get hinds about how to checkout, configure and compile
   lib-ledger-core.
2. You will need `git` and several system dependencies (listed in the [README] document).
3. We work on [GitHub], so you will need to set an SSH and a GPG key:
  1. Generate a public/private key for connecting to [GitHub] via SSH:
    1. `ssh-keygen -t rsa -b 4096`
    2. Enter a path in `/Users/<you>/.ssh`, like for instance, `/Users/<you>/.ssh/github.com.rsa`.
    3. Create a `~/.ssh/config` file in which you need to set `Host` section for github:
       ```
       Host github.com
         HostName github.com
         User git
         IdentityFile ~/.ssh/github.com.rsa
       ````
    4. Copy the content of `~/.ssh/github.com.rsa.pub` — don’t confuse with `github.com.rsa`, which
       is your private key!
    5. Go in your *Account > Settings > SSH and GPG keys* and add the key as *New SSH key*.
  2. Generate a public/private key for signing your commits:
    1. `gpg --full-generate-key`
    2. Choose `rsa` encryption and `4096` bits.
    3. Use your `@ledger.fr` email address.
    4. When done, get the public key summary with `gpg --list-public-keys`.
    5. Copy the long ID under the line starting with `pub`.
    6. `gpg --armor --export <the long ID you copied>`.
    7. Copy the output.
    8. Go in your *Account > Settings > SSH and GPG keys* and add the key as *New GPG key*.
  3. Run the `git config --global user.signingkey <keyid>` — you can find the `<keyid>` using the
     `gpg --list-secret-keys --keyid-format LONG` command: it’s the digest after `rsa4096/`.
  4. `git config --global commit.gpgsign true` will sign all your commits (not only the one you sign
     with `-S` on `git commit`.
  5. You may want to use `gpg-agent --daemonize` so that you don’t have to type your passphrase to
     sign commits every now and then (`brew install gpg-agent`).
4. We have a Jira! You’ll find it [here](https://ledgerhq.atlassian.net/secure/RapidBoard.jspa?rapidView=27&projectKey=LLC&selectedIssue=LLC-6).

[README]: README.md
[GitHub]: https://github.com
