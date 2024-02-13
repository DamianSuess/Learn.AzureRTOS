# Learn Azure RTOS

## Renaming

Note from Eclipse-ThreadX. Some of the links in the Configure Environment are be broken, and the new [Eclipse-ThreadX/Samples](https://github.com/eclipse-threadx/samples) repo doesn't include it.

This repository is a snapshot of the old Azure RTOS Samples repo from 2022-04-06

> **Important:**
>
> We’re pleased to share an important update regarding Azure RTOS – an embedded development suite with the ThreadX real-time operating system that has been deployed on more than 12 billion devices worldwide. Reinforcing our commitment to innovation and community collaboration, during Q1 2024 Azure RTOS will transition to an open-source model. The open-source project is under the stewardship of the Eclipse Foundation, a recognized leader in hosting open-source IoT projects.
>
> With Eclipse Foundation as the new home, Azure RTOS becomes Eclipse ThreadX.
>
> For more information, see the following pages:
>
> * [Microsoft IoT blog](https://techcommunity.microsoft.com/t5/internet-of-things-blog/microsoft-contributes-azure-rtos-to-open-source/ba-p/3986318)
> * [Eclipse Foundation blog](https://eclipse-foundation.blog/2023/11/21/introducing-eclipse-threadx/)
> * [Eclipse ThreadX project](https://threadx.io/)

Transition: _ThreadX > Azure RTOS > Eclipse ThreadX_

## Getting Started

### Directory Layout

```txt
.
├── cmake                        # CMakelist files for building the project
├── docs                         # Documentation supplements
├── courses                      # Source code for learning paths
│   ├── netxduo                  # NetX Duo samples
│   └── threadx                  # ThreadX samples
├── libs                         # Submoduled ThreadX and NetX Duo source code
└── tools                        # Required scripts for using NetX Duo within the container
```

## Configure Environment

[Setup your environment](https://learn.microsoft.com/en-us/training/modules/introduction-azure-rtos/2-set-up-environment) to get started using Visual Studio.

1. Install Visual Studio 2022
   1. Install, **Desktop Development with C++** (2.5 GB)
2. Fork the [Azure-RTOS-Learn-Samples](https://github.com/Azure-Samples/azure-rtos-learn-samples/releases/tag/vs) repo from GitHub
3. Open the main solution
4. Build the projects

## Resources

* [Azure RTOS Documentation](https://learn.microsoft.com/en-us/azure/rtos/threadx/)
* [Azure RTOS ThreadX Training](https://learn.microsoft.com/en-us/training/paths/azure-rtos-threadx/)
* [Azure RTOS Samples](https://github.com/Azure-Samples/azure-rtos-learn-samples)
  * [ZIP: Visual Studio Samples](https://github.com/Azure-Samples/azure-rtos-learn-samples/releases/tag/vs)
  * [BOOK: Real-Time Embedded Multithreadding with ThreadX](https://github.com/Azure-Samples/azure-rtos-learn-samples/releases/download/book/Real-Time_Embedded_Multithreading_with_ThreadX_4th_Edition.pdf)
