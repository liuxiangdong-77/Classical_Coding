# GitHub 上传步骤

## 1. 首次初始化

```powershell
cd E:\github_classic_coding
git init
git branch -M main
```

## 2. 忽略编译目录

```powershell
@'
build/
.vs/
CMakeUserPresets.json
'@ | Set-Content -Encoding utf8 .gitignore
```

## 3. 提交文件

```powershell
git add .
git status
git commit -m "Initial commit"
```

## 4. 关联 GitHub 仓库

```powershell
git remote add origin https://github.com/xiangdongliua/SnakeGame.git
git remote -v
```

将地址替换为你的实际 GitHub 仓库地址。

## 5. 上传

```powershell
git push -u origin main
```

首次上传时，按提示在浏览器登录 GitHub。

## 6. 验证

```powershell
git status
git log --oneline -1
git remote -v
```

## 以后更新代码

```powershell
git status
git add .
git commit -m "描述本次修改"
git push
```
