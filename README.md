<div>
<h1>XMenu</h1>
<p>This is XMenu, a simple app in X11, it uses Xinerama, Xlib and some other libraries.</p>
<p>This is my personal project made for my own Linux distro (Arch, btw). I make it for fun, and I'll update it in the future</p>

<h3>Important</h3>
<p>This project is for WMs DWM-like.</p>
<div/>

### Install

```bash
sudo pacman -S libx11 libxinerama libxft fontconfig imlib2 playerctl curl
```
# Tasks

You can create a task

```bash
XMenu --new-task <Title task> <Short body task>
```
<img width="538" height="221" alt="image" src="https://github.com/user-attachments/assets/3a69b2b7-7800-4346-857f-912400614121" />

Also, you can mark it as complete or not with

```bash
XMenu --task-complete <1|0>
```
<img width="517" height="283" alt="image" src="https://github.com/user-attachments/assets/8da0fb67-a0cb-45d8-9f6b-67fd92caf6e5" />


0 means not completed, and 1 means completed

# Fetch

With this command, you can see your distro, user, hostname, ram, etc. In the app, like neofetch but in C with X11.

```bash
XMenu --fetch
```
<img width="548" height="345" alt="image" src="https://github.com/user-attachments/assets/b291ffbd-d477-468e-a718-496a67b4975e" />
