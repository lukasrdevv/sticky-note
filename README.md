# Native Windows Sticky Note
A lightweight, high-performance sticky note application for Windows 10/11, written in pure C++ using the native Win32 API.

![Sticky Note Icon](logo.ico)

## 🚀 Key Features
*   **Ultra Lightweight**: Minimal resource usage (< 1MB RAM, 0% CPU idle).
*   **Native & Fast**: Built with C++ and Win32 API. No Electron, .NET, or heavy frameworks.
*   **Stealth Mode**: No taskbar entry. Sits quietly on your desktop.
*   **Frameless Design**: Clean, minimal "Post-it" style look.
*   **Smart Positioning**: Automatically places itself in the bottom-left corner of your work area.
*   **Draggable**: Move the note anywhere by dragging the top edge.
*   **Persistent**: Your notes are saved automatically and restored on startup.
*   **System Tray Control**: Toggle visibility or exit via the system tray icon.

## 🛠️ Build Instructions

### Prerequisites
*   Windows 10 or 11
*   **MinGW-w64** (g++ compiler) installed and added to PATH.

### Compiling
Run the included build script:

```cmd
build.bat
```

Or manually compile using g++:

```cmd
windres resource.rc -o resource.o
g++ main.cpp resource.o -o sticky_note.exe -lgdi32 -luser32 -lshell32 -mwindows -O2 -s
```

## 📖 How to Use
1.  **Run** `sticky_note.exe`.
2.  **Write**: Click anywhere in the yellow area to start typing.
    *   Text is saved automatically.
3.  **Move**: Click and drag the **top edge** (header area) of the note to move it.
4.  **Hide/Show**:
    *   Find the **Sticky Note icon** in your System Tray (near the clock).
    *   **Right-Click** the icon to open the menu:
        *   `Hide Note` / `Show Note`
        *   `Exit`

## 📂 Project Structure
*   `main.cpp`: Core application logic (Window management, Message loop, File I/O).
*   `resource.rc` & `resource.h`: Resource scripts for the application icon.
*   `build.bat`: One-click build script.
*   `logo.ico`: Application icon.

## 📝 License
Open Source. Feel free to modify and use.
Icon is from https://www.flaticon.com/free-icon/sticky-notes_3209265