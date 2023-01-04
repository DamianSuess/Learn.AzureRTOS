# Learn Azure RTOS

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
