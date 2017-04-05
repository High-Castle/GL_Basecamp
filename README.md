Notes:
To safely work with git repository you will need following commands. For more specific details about them or git in general 
refer to the git documentation here https://www.atlassian.com/git/tutorials/learn-git-with-bitbucket-cloud 

git clone [link] - pull remote repository using given link.

git checkout -b [new_branch_name] - checkout to a new branch, this branch will contain all changes from origin (branch where you currently are).

Now you can add changes in your repository. Next when you have some set of changes you need to commit.

Firstly use git status, it will list current status of your branch. All files that are changed or untracked (not added to repository yet) 
will be highlighted red. If you will try now to commit, nothing will be commited, you need to add changes to commit.

So you need to call:
git add [file/directory name] (git add . will add all files)

Now if you again use git status command, all files that you are added now highlighted green and can be committed.

git commit -m "[commit message]" - commits changes into current branch.

Finally, to push your changes into remote repository use command:
git push 

That's all, now your changes are in remote repository and you can go into gitlab web page to create pull request to your mentor =).

Some usefull commands:
git status - current branch status
git branch - list all branches, your current branch highlighted green.
git diff - shows in console all changes made in repository that is not commited yet.