# 🖼️ Image Processing Studio

### CS213 — Object-Oriented Programming | Assignment 1 — Part 2

\<p align="center">
&#x20; A C++ image processing application featuring a collection of image filters and a graphical user interface.
\</p>

\<p align="center">
&#x20; \<img src="[https://img.shields.io/badge/C%2B%2B-17-blue?style=for-the-badge&logo=cplusplus&logoColor=white](https://img.shields.io/badge/C%2B%2B-17-blue?style=for-the-badge\&logo=cplusplus\&logoColor=white)" alt="C++17">
&#x20; \<img src="[https://img.shields.io/badge/Platform-Windows-0078D4?style=for-the-badge&logo=windows&logoColor=white](https://img.shields.io/badge/Platform-Windows-0078D4?style=for-the-badge\&logo=windows\&logoColor=white)" alt="Windows">
&#x20; \<img src="[https://img.shields.io/badge/IDE-Visual%20Studio%202022-5C2D91?style=for-the-badge&logo=visualstudio&logoColor=white](https://img.shields.io/badge/IDE-Visual%20Studio%202022-5C2D91?style=for-the-badge\&logo=visualstudio\&logoColor=white)" alt="Visual Studio 2022">
&#x20; \<img src="[https://img.shields.io/badge/Project-CS213-gold?style=for-the-badge](https://img.shields.io/badge/Project-CS213-gold?style=for-the-badge)" alt="CS213 Project">
\</p>

---

## 📌 Overview

**Image Processing Studio** is a team-based C++ project developed for **CS213 — Object-Oriented Programming** at the Faculty of Computers and Artificial Intelligence, Cairo University, under the supervision of **Dr. Mohammad El-Ramly**.

The project explores image processing concepts through a collection of filters, image manipulation operations, and a graphical user interface (GUI). It combines fundamental pixel-level algorithms with an interactive application workflow for loading images, applying effects, and saving the results.

## ✨ Key Features

- **Image Filters:** A collection of 16 core filters, with additional bonus filters developed for the extended version.
- **Graphical User Interface:** An interactive interface for working with images and accessing available operations.
- **Image Loading & Saving:** Load images, apply transformations, and save the processed results.
- **Pixel-Level Processing:** Work directly with image dimensions, color channels, and pixel values.
- **C++ Implementation:** Practice structured programming, functions, image manipulation, and project organization.
- **Team Collaboration:** Collaborative development and source-code management using GitHub.

## 🎨 Available Filters

The project includes the following core filters and bonus features:

| No. | Filter                 | Description                                                                  |
| --: | ---------------------- | ---------------------------------------------------------------------------- |
|   1 | Grayscale              | Converts an image to shades of gray.                                         |
|   2 | Black & White          | Converts an image into black and white using a threshold.                    |
|   3 | Invert Colors          | Inverts the image's color values.                                            |
|   4 | Frame                  | Adds a colored frame around the image.                                       |
|   5 | Flip                   | Flips the image horizontally or vertically, depending on the implementation. |
|   6 | Rotate                 | Rotates an image by a supported angle.                                       |
|   7 | Darken & Lighten       | Adjusts image brightness.                                                    |
|   8 | Resize                 | Changes the image dimensions.                                                |
|   9 | Merge Images           | Combines two images into one result.                                         |
|  10 | Edge Detection         | Highlights edges and boundaries in an image.                                 |
|  11 | Crop                   | Extracts a selected rectangular region of an image.                          |
|  12 | Blur                   | Softens details by averaging nearby pixels.                                  |
|  13 | Sunlight               | Applies a sunlight-inspired color effect.                                    |
|  14 | Old TV                 | Applies a vintage television-inspired effect.                                |
|  15 | Purple Night           | Applies a purple-toned night effect.                                         |
|  16 | Infrared               | Applies an infrared-inspired color effect.                                   |
|  17 | Skew *(Bonus)*         | Applies a skew transformation.                                               |
|  18 | Oil Painting *(Bonus)* | Applies an oil-painting-inspired effect.                                     |

*Filter availability and exact behavior depend on the final integrated version of the application.*

## 🖥️ Interface

The GUI version provides an interactive environment for accessing the image-processing functionality.

**Typical workflow:**

1. Open the application.
2. Load an image.
3. Select an available filter or adjustment.
4. Preview the processed image.
5. Save the result when needed.

> **Screenshots:** Add screenshots of the final GUI here to showcase the interface and sample filter results.

\<!-- Replace the placeholder below with a screenshot committed to your repository. -->

\<!-- Example: ![Application Interface]\(screenshots/main-interface.png) -->

## 🛠️ Technologies Used

- **C++17 — Core programming language.**
- **Microsoft Visual Studio 2026 — Development environment.**
- **Windows SDK & Direct3D 11** — GUI rendering support in the current Windows implementation.
- **Image_Class.h** — Image-processing interface supplied for the assignment.
- **stb_image / stb_image_write** — Image loading and writing support.
- **Dear ImGui** — Immediate-mode GUI framework used by the current GUI implementation.
- **Git & GitHub** — Version control and team collaboration.

## 🚀 Getting Started

### Prerequisites

To build the Windows application, you need:

- Windows.
- Microsoft Visual Studio.
- The **Desktop development with C++** workload.
- A compatible Windows SDK and C++ toolset.
- The required project source files and image-processing headers.

### Run the GUI

1. Clone or download this repository.
2. Open `Image Processing.sln` in Visual Studio 2022.
3. Locate the `PixelLab` project in **Solution Explorer**.
4. Right-click `PixelLab` and select **Set as Startup Project**.
5. Select the `Release` configuration and the `x64` platform.
6. Build the solution.
7. Press **Ctrl + F5** to run the application without debugging.

Visual Studio generates the executable during the build process. Precompiled executable files are not required in the repository.

### Explore the Source Code

Open the main assignment source file:

`CS213_A1_Part2_S7_S8_20250117_20250426_20251551_20253056.cpp`

Use it to review the implemented filter algorithms and the team's source-code submission.

## 📂 Project Structure

```text
Image-Processing/
├── Image Processing.sln
├── Image Processing.vcxproj
├── CS213_A1_Part2_S7_S8_20250117_20250426_20251551_20253056.cpp
├── main.cpp
├── Image_Class.h
├── stb_image.h
├── stb_image_write.h
├── README.md
├── Links.txt
└── PixelLab/
    ├── PixelLab.vcxproj
    ├── GUI.cpp
    ├── AppState.h
    ├── Adjustments.cpp
    ├── Adjustments.h
    ├── FilterBridge.cpp
    ├── FilterBridge.h
    ├── Renderer.cpp
    ├── Renderer.h
    ├── Theme.h
    └── imgui/
```

*The structure above is illustrative. Actual filenames and folders may differ in the final repository.*

## 👥 Team Members & Contributions

| Team Member             | Student ID | Assigned Filters |
| ----------------------- | ---------: | ---------------- |
| Belal Ibrahim Sayed     |   20250117 | 1, 5, 9, 13      |
| Omar Hossam Eldin Zidan |   20250426 | 2, 6, 10, 14     |
| Ahmed Mahmoud Roshdy    |   20251551 | 3, 7, 11, 15     |
| Shahd Ali Mohammed      |   20253056 | 4, 8, 12, 16     |

**Section:** S7, S8

The team collaborated on integrating the individual filters into a unified application and preparing the project for demonstration and submission.

## 🎓 Academic Context

- **Course:** CS213 — Object-Oriented Programming
- **Assignment:** Assignment 1 — Part 2
- **Institution:** Faculty of Computers and Artificial Intelligence, Cairo University
- **Supervisor:** Dr. Mohammad El-Ramly
- **Academic Year:** 2026–2027

This project was developed as part of the course's practical programming work.

## 🎥 Demo & Documentation

- **Video Demonstration:** Add the public video link here.
- **Shared Project Documentation:** Add the shared document link here.

The demonstration should introduce the team and course, explain how to build and run the application, and show the available filters and their results.

## 📚 Learning Outcomes

Through this project, the team practiced:

- Translating image-processing concepts into algorithms.
- Manipulating RGB pixel data and image dimensions.
- Organizing C++ code into functions and source files.
- Integrating multiple features into a unified application.
- Building a graphical interface for an existing processing workflow.
- Collaborating through GitHub and integrating individual contributions.
- Testing image transformations and handling user input.

## ⚠️ Notes

- This repository is intended for academic and educational use as part of CS213.
- The project must be built locally using a compatible development environment.
- No compiled `.exe` files or generated build artifacts are intended to be committed.
- Any third-party source code included in the repository should follow its applicable license and the course submission requirements.

---

\<p align="center">
&#x20; \<b>Developed by the CS213 Project Team\</b>\<br>
&#x20; Faculty of Computers and Artificial Intelligence — Cairo University\<br>
&#x20; Academic Year 2026–2027
\</p>
