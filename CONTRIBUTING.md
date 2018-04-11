# Contributing to Notepadqq

Thanks for helping to make Notepadqq better! As a contributor, here are the guidelines we would like you to follow:

 - [Question or Problem?](#question)
 - [Issues and Bugs](#issue)
 - [Feature Requests](#feature)
 - [Submission Guidelines](#submit)
 - [Coding Rules](#rules)

## <a name="question"></a> Got a Question or Problem?

Please, do not open issues for the general support questions as we want to keep GitHub issues for bug reports and feature requests. You've got much better chances of getting your question answered on [Google+](https://plus.google.com/communities/118430810002505082315).

## <a name="issue"></a> Found a Bug?
If you find a bug in the source code, you can help us by
[submitting an issue](#submit-issue) to our [GitHub Repository][github]. Even better, you can
[submit a Pull Request](#submit-pr) with a fix.

## <a name="feature"></a> Missing a Feature?
You can *request* a new feature by [submitting an issue](#submit-issue) to our GitHub
Repository. If you would like to *implement* a new feature, please submit an issue with
a proposal for your work first, to be sure that we can use it.
Please consider what kind of change it is:

* For a **Major Feature**, first open an issue and outline your proposal so that it can be
discussed. This will also allow us to better coordinate our efforts, prevent duplication of work,
and help you to craft the change so that it is successfully accepted into the project.
* **Small Features** can be crafted and directly [submitted as a Pull Request](#submit-pr).

## <a name="submit"></a> Submission Guidelines

### <a name="submit-issue"></a> Submitting an Issue

Before you submit an issue, please search the issue tracker, maybe an issue for your problem already exists and the discussion might inform you of workarounds readily available.

We want to fix all the issues as soon as possible, but before fixing a bug we need to reproduce and confirm it. In order to reproduce bugs you should explain the exact steps you performed to make the problem show up. In addition, you should always specify:

- version of your operating system
- version of Qt (menu `? -> About Qt...`)
- version of Notepadqq (menu `? -> About Notepadqq...`)

You can file new issues by filling out our [new issue form](https://github.com/notepadqq/notepadqq/issues/new).


### <a name="submit-pr"></a> Submitting a Pull Request (PR)
Before you submit your Pull Request (PR) consider the following guidelines:

* Search [GitHub](https://github.com/notepadqq/notepadqq/pulls) for an open or closed PR
  that relates to your submission. You don't want to duplicate effort.
* Create a new git branch in which you'll make your changes:

     ```shell
     git checkout -b my-fix-branch master
     ```

* Create your patch.
* Take some time to review the diff of your changes, so that you're confident that everything is correct.
* Commit your changes using a descriptive commit message.

     ```shell
     git commit -a
     ```
  Note: the optional commit `-a` command line option will automatically "add" and "rm" edited files.

* Push your branch to GitHub:

    ```shell
    git push origin my-fix-branch
    ```

* In GitHub, send a pull request to `notepadqq:master`. Always include an explanation about *what* your code does and *why*.
* If we suggest changes then:
  * Make the required updates.
  * Rebase your branch and force push to your GitHub repository (this will update your Pull Request):

    ```shell
    git rebase master -i
    git push -f
    ```

That's it! Thank you for your contribution!

[Learn more about the git workflow](https://gist.github.com/Chaser324/ce0505fbed06b947d962)

#### After your pull request is merged

After your pull request is merged, you can safely delete your branch and pull the changes
from the main (upstream) repository:

* Delete the remote branch on GitHub either through the GitHub web UI or your local shell as follows:

    ```shell
    git push origin --delete my-fix-branch
    ```

* Check out the master branch:

    ```shell
    git checkout master -f
    ```

* Delete the local branch:

    ```shell
    git branch -D my-fix-branch
    ```

* Update your master with the latest upstream version:

    ```shell
    git pull --ff upstream master
    ```

## <a name="rules"></a> Coding Rules
To ensure consistency throughout the source code, keep these rules in mind as you are working:

 * Make sure that your code respects the style conventions of the project. Look at the code around you: do you see
   spaces before parenthesis? Are brackets on their own line? Copy that! The most important thing is to be consistent.
 * Put a comment block over your methods, describing what they do, their parameters, and their results.
 * One pull request is meant to contain just one fix/feature. Open another pull request if you have some other
   unrelated change to submit.
 * Make sure your pull request doesn't contain trivial, unwanted changes. For example, if you're working on a big feature
   and you happen to open Qt Designer and it increases the height of the window by 1px, please keep that out of the pull request.
   In general, keep the number of changes at the minimum necessary.


[github]: https://github.com/notepadqq/notepadqq
