# FSR4 Desktop Upscaler (Direct3D 11)

Prototype Windows desktop application that captures the primary monitor via DXGI Desktop Duplication, upscales the frame using AMD FidelityFX Super Resolution (FSR4 placeholder hooks), and displays the result in a Win32 window with a full-screen triangle.

## Layout
- `src/main.cpp` — Win32 entry point and message loop.
- `src/App.h/cpp` — application state, D3D11/duplication initialization, per-frame capture/upscale/blit.
- `src/Fsr4Wrapper.h/cpp` — isolated FidelityFX upscaler integration points with TODOs for the real SDK API.
- `shaders/*.hlsl` — minimal full-screen triangle shaders.
- `CMakeLists.txt` — CMake build (set the FidelityFX SDK path in include directories / linking).

## FidelityFX SDK integration
1. Download/extract FidelityFX SDK 2.1.0 somewhere like `external/FidelityFX-SDK-2.1.0`.
2. Update `CMakeLists.txt` include paths and add/link the appropriate FSR4 source or library targets.
3. Replace the TODO placeholder API calls in `Fsr4Wrapper.*` with the actual types/functions from the SDK (e.g., `FfxFsrContext`, `ffxFsrContextCreate`, `ffxFsrContextDispatch`).

## Building (CMake example)
```bash
cmake -S cpp-fsr4-sample -B build -G "Visual Studio 17 2022" -A x64 \
  -DFFX_SDK_ROOT="C:/path/to/external/FidelityFX-SDK-2.1.0"
cmake --build build --config Release
```
Adjust generator/paths as needed; the sample links only against Windows SDK libraries.

## Step-by-step: turn this into a runnable app
1. **Point CMake at the SDK**
   - Open `CMakeLists.txt` and replace the TODO include path with your extracted SDK include directory, e.g. `external/FidelityFX-SDK-2.1.0/include`.
   - Add the required SDK sources or prebuilt libs for the FSR4/FSR upscaler targets (follow the SDK's CMake add_subdirectory guidance if available).

2. **Wire the wrapper to the actual API**
   - In `src/Fsr4Wrapper.cpp/.h`, swap the placeholder structs/functions for the real FidelityFX FSR4 names from 2.1.0 (context creation, dispatch, and destroy). All TODO markers indicate where to patch signatures.
   - Make sure the wrapper's create/destroy/dispatch calls match the SDK's required interface callbacks (allocator, error callbacks, etc.) and device/context handles.

3. **Build with Visual Studio or CMake**
   - Generate a VS solution with the command above or open the folder in Visual Studio and select the `Fsr4DesktopUpscaler` target.
   - Ensure the Windows SDK is installed (D3D11/DXGI). Build `Release` x64 for best performance.

4. **Run the app**
   - The executable expects shader files in a `shaders/` folder next to the binary; the post-build copy step in `CMakeLists.txt` handles this automatically.
   - Launch the app: it creates a resizable window, captures the primary monitor via DXGI Desktop Duplication, upscales the captured frame through your FSR integration, and presents the upscaled result via a full-screen triangle blit.

5. **Debugging tips**
   - If capture stalls, check for `DXGI_ERROR_WAIT_TIMEOUT` handling in `App::CaptureFrame`.
   - Validation failures during upscaling typically mean the FSR dispatch description does not match the SDK's expected resource views or sizes; log or assert around the TODO sections when you wire the real API.

With these steps, you can swap the placeholder FidelityFX calls for the actual SDK symbols, build the project, and run a desktop upscaler prototype end-to-end.
