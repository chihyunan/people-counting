EECS 300 People Counting Project
================================
Two IR laser beam sensors and One Thermal Grideye Camera 

By Team 14, W26 <> 

Current high-level structure
<img width="721" height="311" alt="image" src="https://github.com/user-attachments/assets/853283d9-9655-4d2b-90eb-a987737ef844" />

## Project Description

### Materials
ESP32,
Sensors: 

## Code - How it Works 

### 🛠 Part 1: One-Time Setup
Each teammate runs this once to align their machines with the system.

Install GitHub CLI:
Mac: `brew install gh`
Windows: `winget install --id GitHub.cli`

Authenticate:
`gh auth login (Follow the browser prompts)`

Set the "Straight Line" Rule:
`git config --global pull.rebase true`

### 🚀 Part 2: The Daily Workflow
Follow this every time you write code. No exceptions.

1. Start Fresh
Never build on old code.

```bash
git checkout main
git pull origin main
```

2. Create a Feature Branch
Give it a name that makes sense (e.g., ir-sensor-fix).

`git checkout -b <branch-name>`

3. Code & Ship
Write your C/C++ code. Once it works locally:

```
git add .
git commit -m "feat: updated sensor logic"
git push origin <branch-name>
```

4. The "Magic" Merge
Run these two commands. The machine (ESP32 Compiler) will check your work.

```
gh pr create --fill
gh pr merge --auto --squash
```
Note: If the "Merge" button on GitHub is red, it means your code failed to compile. Click the link in the terminal to see the error, fix it, and git push again.

Other: 
The Library Rule: If you add a new #include <Library.h>, did you also add it to the .github/workflows/verify.yml? (If not, the CI will kill your PR).

## Licensing
This repository is a Fork/Derivative.

Original Logic: [zhaochenyuangit] (No License Provided).
https://github.com/zhaochenyuangit/IR-room-monitor

Our Enhancements: Licensed under MIT.

Contribution Policy: By contributing to this repo, you agree to license your work under MIT.
