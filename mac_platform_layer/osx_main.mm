/*

    Texture Editor Mac Platform Layer
    2020 TedBendixson
    Mooselutions, LLC

    Use this as a reference for your own texture editor tool.
*/

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <QuartzCore/CoreAnimation.h>
#import <IOKit/hid/IOHIDLib.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <mach/mach_init.h>
#include <mach/mach_time.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <sys/stat.h>

#include "../cross_platform/texture_editor_types.h"
#include "../cross_platform/texture_editor.cpp"
#include "../cross_platform/texture_editor_strings.h"

#include "osx_main.h"

global_variable MTKView *MetalKitView;

void
MacBuildAppFilePath(mac_app_path *Path)
{
	uint32 buffsize = sizeof(Path->Filename);
    if (_NSGetExecutablePath(Path->Filename, &buffsize) == 0) {
		for(char *Scan = Path->Filename;
			*Scan;
			++Scan)
		{
			if(*Scan == '/')
			{
				Path->OnePastLastAppFileNameSlash = Scan + 1;
			}
		}
    }
}

void
MacBuildAppPathFileName(mac_app_path *Path, char *Filename, int DestCount, char *Dest)
{
	CatStrings(Path->OnePastLastAppFileNameSlash - Path->Filename, Path->Filename,
			   StringLength(Filename), Filename,
			   DestCount, Dest);
}

#if INTERNAL
void PlatformFreeFileMemory(void *Memory)
{
    if (Memory) {
        free(Memory);
    }
}

// NOTE: (Ted)  For some reason, this is the only way to load a level from a file
//              dialog without any data corruption. I think this is because it bypasses
//              some sort of finder entitlements nonsense.
internal
read_file_result MacReadEntireFileFromDialog(char *Filename)
{
    read_file_result Result = {};

    FILE *FileHandle = fopen(Filename, "r+");

    if(FileHandle != NULL)
    {
		fseek(FileHandle, 0, SEEK_END);
		uint64 FileSize = ftell(FileHandle);
        if(FileSize)
        {
        	rewind(FileHandle);
            Result.Contents = malloc(FileSize);

            if(Result.Contents)
            {
                uint64 BytesRead = fread(Result.Contents, 1, FileSize, FileHandle);
                if(FileSize == BytesRead)
                {
                    Result.ContentsSize = FileSize;
                    Result.Filename = (char *)malloc(200*sizeof(char));

                    char *Dest = Result.Filename;
                    char *Scan = Filename;

                    while (*Scan != '\0')
                    {
                        *Dest++ = *Scan++;
                    }

                    *Dest++ = '\0';
                }
                else
                {                    
                    NSLog(@"File loaded size mismatch. FileSize: %llu, BytesRead: %llu",
                          FileSize, BytesRead);
                    PlatformFreeFileMemory(Result.Contents);
                    PlatformFreeFileMemory(Result.Filename);
                    Result.Contents = 0;
                }
            }
            else
            {
                NSLog(@"Missing Result Contents Pointer from file load.");
            }
        }
        else
        {
            NSLog(@"Missing File Size from file load");
        }

        fclose(FileHandle);
    }
    else
    {
        NSLog(@"Unable to acquire File handle");
    }

    return (Result);
}

internal
char * ConvertAbsoluteURLToFileURL(NSURL *FileURL)
{
    NSMutableString *FilePath = [[FileURL absoluteString] mutableCopy];
    [FilePath replaceOccurrencesOfString: @"file://" 
                              withString: @""
                                 options: NSCaseInsensitiveSearch
                                   range: NSMakeRange(0,7)]; 
    char *LocalFilename = (char *)[FilePath cStringUsingEncoding: NSUTF8StringEncoding];
    [FilePath release];
    return (LocalFilename);

}

read_file_result PlatformOpenFileDialog()
{
    read_file_result Result = {};

    @autoreleasepool
    {
        NSOpenPanel *OpenPanel = [[[NSOpenPanel alloc] init] autorelease];
        OpenPanel.canChooseFiles = true;
        OpenPanel.canChooseDirectories = true;
        OpenPanel.allowsMultipleSelection = false;

        if ([OpenPanel runModal] == NSModalResponseOK)
        {
            NSURL *FileURL = [[OpenPanel URLs] objectAtIndex: 0];
            char *LocalFilename = ConvertAbsoluteURLToFileURL(FileURL);
            Result = MacReadEntireFileFromDialog(LocalFilename);
        }
    }

    return (Result);
}

read_file_result PlatformReadEntireFile(char *Filename)
{
    read_file_result Result = {};
    Result.Filename = Filename;

    mac_app_path Path = {};
    MacBuildAppFilePath(&Path);

    char SandboxFilename[MAC_MAX_FILENAME_SIZE];
    char LocalFilename[MAC_MAX_FILENAME_SIZE];
    sprintf(LocalFilename, "Contents/Resources/%s", Filename);
    MacBuildAppPathFileName(&Path, LocalFilename,
                            sizeof(SandboxFilename), SandboxFilename);

    FILE *FileHandle = fopen(SandboxFilename, "r+");

    if(FileHandle != NULL)
    {
		fseek(FileHandle, 0, SEEK_END);
		uint64 FileSize = ftell(FileHandle);
        if(FileSize)
        {
        	rewind(FileHandle);
        	Result.Contents = malloc(FileSize);
            if(Result.Contents)
            {
                uint64 BytesRead = fread(Result.Contents, 1, FileSize, FileHandle);
                if(FileSize == BytesRead)
                {
                    Result.ContentsSize = FileSize;
                }
                else
                {                    
                    NSLog(@"Failed to read a file from the file system. The file size did not match.");
                    PlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                NSLog(@"Failed to read a file from the file system. Zero contents.");
            }
        }
        else
        {
            NSLog(@"Failed to read a file from the file system. No file size.");
        }

        fclose(FileHandle);
    }
    else
    {
        NSLog(@"Failed to read a file from the file system. File Handle is Null.");
    }

    return(Result);
}

@interface TextureEditorSavePanelDelegate: NSObject<NSOpenSavePanelDelegate>
@end

@implementation TextureEditorSavePanelDelegate 

- (BOOL)panel:(id)sender shouldEnableURL:(NSURL *)url {
    return true;
}

@end

bool32 PlatformWriteEntireFile(uint64 FileSize, void *Memory)
{
    bool32 Result = false;

    @autoreleasepool
    {
        NSSavePanel *SavePanel = [[[NSSavePanel alloc] init] autorelease];

        TextureEditorSavePanelDelegate  *SavePanelDelegate = [[[TextureEditorSavePanelDelegate alloc] init] autorelease];
        [SavePanel setDelegate: SavePanelDelegate];
        SavePanel.title = @"Save Game Texture";
        SavePanel.prompt = @"Save";
        SavePanel.canCreateDirectories = true;

        if ([SavePanel runModal] == NSModalResponseOK)
        {
            NSMutableString * FilePath = [[SavePanel.URL absoluteString] mutableCopy];
            [FilePath replaceOccurrencesOfString: @"file://" 
                                      withString:@""
                                         options: NSCaseInsensitiveSearch
                                           range: NSMakeRange(0,7)]; 

            char *LocalFilename = (char *)[FilePath cStringUsingEncoding: NSUTF8StringEncoding];

            FILE *FileHandle = fopen(LocalFilename, "w");

            if(FileHandle)
            {
                size_t BytesWritten = fwrite(Memory, 1, FileSize, FileHandle);
                if(BytesWritten)
                {
                    Result = (BytesWritten == FileSize);
                }
                else
                {
                    NSLog(@"No bytes written to the file system.");
                }

                fclose(FileHandle);
            }
            else
            {
                NSLog(@"No file written. Unable to obtain file handle");
            }

        } else
        {
            Result = false;
        }

    }

    return(Result);
}

#endif


@interface MainWindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation MainWindowDelegate 

- (void)windowWillClose:(id)sender 
{
    [NSApp performSelector: @selector(terminate:) withObject: nil afterDelay: 0.0];
}

@end

@interface TextureEditorWindow: NSWindow

-(void) setKeyboardInputPtr:(texture_editor_keyboard_input *)KeyboardInputPtr;

@end

@implementation TextureEditorWindow
{
    texture_editor_keyboard_input *_KeyboardInputPtr;
}

-(void) setKeyboardInputPtr:(texture_editor_keyboard_input *)KeyboardInputPtr
{
    _KeyboardInputPtr = KeyboardInputPtr;
}

- (void)keyDown:(NSEvent *)theEvent 
{ 
    if (theEvent.keyCode == AKeyCode)
    {
        _KeyboardInputPtr->A.EndedDown = true;
    } 
    else if (theEvent.keyCode == CKeyCode)
    {
        _KeyboardInputPtr->C.EndedDown = true;
    } 
    else if (theEvent.keyCode == SKeyCode)
    {
        _KeyboardInputPtr->S.EndedDown = true;
    }
    else if (theEvent.keyCode == FKeyCode)
    {
        _KeyboardInputPtr->F.EndedDown = true;
    }
    else if (theEvent.keyCode == PKeyCode)
    {
        _KeyboardInputPtr->P.EndedDown = true;
    }
    else if (theEvent.keyCode == YKeyCode)
    {
        _KeyboardInputPtr->Y.EndedDown = true;
    }
    else if (theEvent.keyCode == HKeyCode)
    {
        _KeyboardInputPtr->H.EndedDown = true;
    }
    else if (theEvent.keyCode == JKeyCode)
    {
        _KeyboardInputPtr->J.EndedDown = true;
    }
    else if (theEvent.keyCode == KKeyCode)
    {
        _KeyboardInputPtr->K.EndedDown = true;
    }
    else if (theEvent.keyCode == LKeyCode)
    {
        _KeyboardInputPtr->L.EndedDown = true;
    }
    else if (theEvent.keyCode == MKeyCode)
    {
        _KeyboardInputPtr->M.EndedDown = true;
    }
    else if (theEvent.keyCode == NKeyCode)
    {
        _KeyboardInputPtr->N.EndedDown = true;
    }
    else if (theEvent.keyCode == LeftArrowKeyCode)
    {
        _KeyboardInputPtr->LeftArrow.EndedDown = true;
    }
    else if (theEvent.keyCode == RightArrowKeyCode)
    {
        _KeyboardInputPtr->RightArrow.EndedDown = true;
    }
    else if (theEvent.keyCode == DownArrowKeyCode)
    {
        _KeyboardInputPtr->DownArrow.EndedDown = true;
    }
    else if (theEvent.keyCode == UpArrowKeyCode)
    {
        _KeyboardInputPtr->UpArrow.EndedDown = true;
    }
    else if (theEvent.keyCode == F1KeyCode)
    {
        _KeyboardInputPtr->F1.EndedDown = true;
    }
    else if (theEvent.keyCode == F2KeyCode)
    {
        _KeyboardInputPtr->F2.EndedDown = true;
    }
    else if (theEvent.keyCode == F4KeyCode)
    {
        _KeyboardInputPtr->F4.EndedDown = true;
    }
    else if (theEvent.keyCode == F5KeyCode)
    {
        _KeyboardInputPtr->F5.EndedDown = true;
    }
    else if (theEvent.keyCode == F6KeyCode)
    {
        _KeyboardInputPtr->F6.EndedDown = true;
    }
    else if (theEvent.keyCode == F7KeyCode)
    {
        _KeyboardInputPtr->F7.EndedDown = true;
    }
    else if (theEvent.keyCode == ReturnKeyCode)
    {
        _KeyboardInputPtr->Return.EndedDown = true;
    }
}

- (void)keyUp:(NSEvent *)theEvent
{
    if (theEvent.keyCode == AKeyCode)
    {
        _KeyboardInputPtr->A.EndedDown = false;
    } 
    else if (theEvent.keyCode == CKeyCode)
    {
        _KeyboardInputPtr->C.EndedDown = false;
    } 
    else if (theEvent.keyCode == SKeyCode)
    {
        _KeyboardInputPtr->S.EndedDown = false;
    }
    else if (theEvent.keyCode == FKeyCode)
    {
        _KeyboardInputPtr->F.EndedDown = false;
    }
    else if (theEvent.keyCode == PKeyCode)
    {
        _KeyboardInputPtr->P.EndedDown = false;
    }
    else if (theEvent.keyCode == YKeyCode)
    {
        _KeyboardInputPtr->Y.EndedDown = false;
    }
    else if (theEvent.keyCode == HKeyCode)
    {
        _KeyboardInputPtr->H.EndedDown = false;
    }
    else if (theEvent.keyCode == JKeyCode)
    {
        _KeyboardInputPtr->J.EndedDown = false;
    }
    else if (theEvent.keyCode == KKeyCode)
    {
        _KeyboardInputPtr->K.EndedDown = false;
    }
    else if (theEvent.keyCode == LKeyCode)
    {
        _KeyboardInputPtr->L.EndedDown = false;
    }
    else if (theEvent.keyCode == MKeyCode)
    {
        _KeyboardInputPtr->M.EndedDown = false;
    }
    else if (theEvent.keyCode == NKeyCode)
    {
        _KeyboardInputPtr->N.EndedDown = false;
    }
    else if (theEvent.keyCode == LeftArrowKeyCode)
    {
        _KeyboardInputPtr->LeftArrow.EndedDown = false;
    }
    else if (theEvent.keyCode == RightArrowKeyCode)
    {
        _KeyboardInputPtr->RightArrow.EndedDown = false;
    }
    else if (theEvent.keyCode == DownArrowKeyCode)
    {
        _KeyboardInputPtr->DownArrow.EndedDown = false;
    }
    else if (theEvent.keyCode == UpArrowKeyCode)
    {
        _KeyboardInputPtr->UpArrow.EndedDown = false;
    }
    else if (theEvent.keyCode == F1KeyCode)
    {
        _KeyboardInputPtr->F1.EndedDown = false;
    }
    else if (theEvent.keyCode == F2KeyCode)
    {
        _KeyboardInputPtr->F2.EndedDown = false;
    }
    else if (theEvent.keyCode == F4KeyCode)
    {
        _KeyboardInputPtr->F4.EndedDown = false;
    }
    else if (theEvent.keyCode == F5KeyCode)
    {
        _KeyboardInputPtr->F5.EndedDown = false;
    }
    else if (theEvent.keyCode == F6KeyCode)
    {
        _KeyboardInputPtr->F6.EndedDown = false;
    }
    else if (theEvent.keyCode == F7KeyCode)
    {
        _KeyboardInputPtr->F7.EndedDown = false;
    }
    else if (theEvent.keyCode == ReturnKeyCode)
    {
        _KeyboardInputPtr->Return.EndedDown = false;
    }
}

@end

internal void
MacProcessGameControllerButton(texture_editor_button_state *OldState, texture_editor_button_state *NewState,
                               bool32 IsDown) 
{
    NewState->EndedDown = IsDown;
    NewState->HalfTransitionCount += ((NewState->EndedDown == OldState->EndedDown)?0:1);
}

@interface MetalViewDelegate: NSObject<MTKViewDelegate>

@property texture_editor_render_commands RenderCommands;
@property texture_editor_memory EditorMemory;
@property texture_editor_state EditorState;

@property (retain) id<MTLRenderPipelineState> ColorPipelineState;
@property (retain) id<MTLRenderPipelineState> TexturePipelineState;
@property (retain) id<MTLCommandQueue> CommandQueue;
@property (retain) NSMutableArray* ColorVertexBuffers;
@property (retain) NSMutableArray* TextureVertexBuffers;
@property (retain) id<MTLTexture> PreviewBitmapTexture;

- (void)configureMetal;
- (void) setGameControllerPtr: (mac_game_controller *)GameControllerPtr;
- (void) setKeyboardInputPtr:(texture_editor_keyboard_input *)KeyboardInputPtr;
- (void) setMacStatePtr: (mac_state *)MacStatePtr;
- (void) setNewInputPtr: (texture_editor_input *)NewInputPtr;
- (void) setOldInputPtr: (texture_editor_input *)OldInputPtr;

@end

static const NSUInteger kMaxInflightBuffers = 3;

@implementation MetalViewDelegate
{
    mac_game_controller *_GameControllerPtr;
    texture_editor_keyboard_input *_KeyboardInputPtr;
    mac_state *_MacStatePtr;
    texture_editor_input *_NewInputPtr;
    texture_editor_input *_OldInputPtr;

    dispatch_semaphore_t _frameBoundarySemaphore;
    NSUInteger _currentFrameIndex;
}

- (void)configureMetal
{
    _frameBoundarySemaphore = dispatch_semaphore_create(kMaxInflightBuffers);
    _currentFrameIndex = 0;
}

- (void) setGameControllerPtr: (mac_game_controller *)GameControllerPtr
{
    _GameControllerPtr = GameControllerPtr;
}

-(void) setKeyboardInputPtr:(texture_editor_keyboard_input *)KeyboardInputPtr
{
    _KeyboardInputPtr = KeyboardInputPtr;
}

- (void) setMacStatePtr: (mac_state *)MacStatePtr
{
    _MacStatePtr = MacStatePtr;
}

- (void) setNewInputPtr: (texture_editor_input *)NewInputPtr
{
    _NewInputPtr = NewInputPtr;
}

- (void) setOldInputPtr: (texture_editor_input *)OldInputPtr
{
    _OldInputPtr = OldInputPtr;
}

- (void)drawInMTKView:(MTKView *)view 
{
    dispatch_semaphore_wait(_frameBoundarySemaphore, DISPATCH_TIME_FOREVER);

    _currentFrameIndex = (_currentFrameIndex + 1) % kMaxInflightBuffers;

    texture_editor_render_commands *RenderCommandsPtr = &_RenderCommands;
    RenderCommandsPtr->FrameIndex = (uint32)_currentFrameIndex;

    _NewInputPtr->dtForFrame = 1.0f/60.0f;

    mac_game_controller *MacController = _GameControllerPtr;

    texture_editor_controller_input *OldController = &_OldInputPtr->GameController;
    texture_editor_controller_input *NewController = &_NewInputPtr->GameController;

    MacProcessGameControllerButton(&(OldController->A),
                                     &(NewController->A),
                                     MacController->ButtonAState); 

    MacProcessGameControllerButton(&(OldController->B),
                                     &(NewController->B),
                                     MacController->ButtonBState); 

    MacProcessGameControllerButton(&(OldController->X),
                                     &(NewController->X),
                                     MacController->ButtonXState); 

    MacProcessGameControllerButton(&(OldController->Y),
                                     &(NewController->Y),
                                     MacController->ButtonYState); 

    MacProcessGameControllerButton(&(OldController->LeftShoulder),
                                     &(NewController->LeftShoulder),
                                     MacController->ButtonLeftShoulderState); 

    MacProcessGameControllerButton(&(OldController->LeftShoulder2),
                                     &(NewController->LeftShoulder2),
                                     MacController->ButtonLeftShoulder2State); 
   
    MacProcessGameControllerButton(&(OldController->RightShoulder),
                                     &(NewController->RightShoulder),
                                     MacController->ButtonRightShoulderState); 

    MacProcessGameControllerButton(&(OldController->RightShoulder2),
                                     &(NewController->RightShoulder2),
                                     MacController->ButtonRightShoulder2State); 

    MacProcessGameControllerButton(&(OldController->Select),
                                     &(NewController->Select),
                                     MacController->ButtonSelectState); 

    MacProcessGameControllerButton(&(OldController->Start),
                                     &(NewController->Start),
                                     MacController->ButtonStartState); 

    bool32 Right = MacController->DPadX > 0 ? true:false;
    bool32 Left = MacController->DPadX < 0 ? true:false;
    bool32 Up = MacController->DPadY > 0 ? true:false;
    bool32 Down = MacController->DPadY < 0 ? true:false;

    MacProcessGameControllerButton(&(OldController->Right),
                                   &(NewController->Right),
                                   Right);
    MacProcessGameControllerButton(&(OldController->Left),
                                   &(NewController->Left),
                                   Left);
    MacProcessGameControllerButton(&(OldController->Up),
                                   &(NewController->Up),
                                   Up);
    MacProcessGameControllerButton(&(OldController->Down),
                                   &(NewController->Down),
                                   Down);

    NewController->IsAnalog = MacController->UsesHatSwitch;
    NewController->StartX = OldController->EndX;
    NewController->StartY = OldController->EndY;

    NewController->EndX = (real32)(MacController->LeftThumbstickX - 127.5f)/127.5f;
    NewController->EndY = (real32)(MacController->LeftThumbstickY - 127.5f)/127.5f;

    NewController->MinX = NewController->MaxX = NewController->EndX;            
    NewController->MinY = NewController->MaxY = NewController->EndY;            

    real32 DeadZone = 0.15f;
    real32 ScalarEndX = abs(NewController->EndX);
    real32 ScalarEndY = abs(NewController->EndY);

    if (ScalarEndX < DeadZone)
    {
        NewController->EndX = 0.0f;
    }

    if (ScalarEndY < DeadZone)
    {
        NewController->EndY = 0.0f;
    }

    if (MacController->UsesHatSwitch)
    {
        NewController->IsAnalog = OldController->IsAnalog;

        if (ScalarEndX > DeadZone || ScalarEndY > DeadZone)
        {
            NewController->IsAnalog = true;
        } else 
        {
            NewController->IsAnalog = false;
        }
    }
  
    color_vertex_command_buffer ColorCommandBuffer = RenderCommandsPtr->ColorVertexCommandBuffers[RenderCommandsPtr->FrameIndex];
    ColorCommandBuffer.NumberOfColorVertices = 0;
    RenderCommandsPtr->ColorVertexCommandBuffers[RenderCommandsPtr->FrameIndex] = ColorCommandBuffer;

    texture_vertex_command_buffer TextureCommandBuffer = RenderCommandsPtr->TextureVertexCommandBuffers[RenderCommandsPtr->FrameIndex];
    TextureCommandBuffer.NumberOfTextureVertices = 0;
    RenderCommandsPtr->TextureVertexCommandBuffers[RenderCommandsPtr->FrameIndex] = TextureCommandBuffer;

    texture_editor_memory *EditorMemoryPtr = &_EditorMemory;

    UpdateAndRender(EditorMemoryPtr, _NewInputPtr, 
                    _KeyboardInputPtr, RenderCommandsPtr);

    _KeyboardInputPtr->F1.EndedDown = false;
    _KeyboardInputPtr->F2.EndedDown = false;

    texture_editor_input *Temp = _NewInputPtr;
    _NewInputPtr = _OldInputPtr;
    _OldInputPtr = Temp;

    NSUInteger Width = (NSUInteger)RenderCommandsPtr->ViewportWidth*2;
    NSUInteger Height = (NSUInteger)RenderCommandsPtr->ViewportHeight*2;
    MTLViewport Viewport = (MTLViewport){0.0, 0.0, (real64)Width, (real64)Height, -1.0, 1.0 };

    NSUInteger BytesPerRow = (NSUInteger)RenderCommandsPtr->PreviewTextureWidth*sizeof(uint32);
    NSUInteger TextureWidth = (NSUInteger)RenderCommandsPtr->PreviewTextureWidth;
    NSUInteger TextureHeight = (NSUInteger)RenderCommandsPtr->PreviewTextureHeight;

    MTLRegion MetalRegion = {
        { 0, 0, 0 },
        { TextureWidth, TextureHeight, 1 }
    };

    @autoreleasepool {

        texture_editor_state *EditorStatePtr = (texture_editor_state *)EditorMemoryPtr->PermanentStorage;

        [[self PreviewBitmapTexture] replaceRegion: MetalRegion
                                       mipmapLevel: 0
                                         withBytes: (void *)EditorStatePtr->PixelBuffer.Pixels
                                       bytesPerRow: BytesPerRow];

        id<MTLCommandBuffer> CommandBuffer = [[self CommandQueue] commandBuffer];
        MTLRenderPassDescriptor *RenderPassDescriptor = [view currentRenderPassDescriptor];
        vector_uint2 ViewportSize = { (uint32)RenderCommandsPtr->ViewportWidth, 
                                      (uint32)RenderCommandsPtr->ViewportHeight };

        ColorCommandBuffer = RenderCommandsPtr->ColorVertexCommandBuffers[RenderCommandsPtr->FrameIndex];
        NSUInteger NumberOfColorVertices = (NSUInteger)ColorCommandBuffer.NumberOfColorVertices;

        id<MTLRenderCommandEncoder> RenderEncoder = [CommandBuffer renderCommandEncoderWithDescriptor:RenderPassDescriptor];

        [RenderEncoder setViewport: Viewport];

        [RenderEncoder setRenderPipelineState: [self ColorPipelineState]];

        [RenderEncoder setVertexBuffer: [[self ColorVertexBuffers] objectAtIndex: _currentFrameIndex]
                                offset: 0
                               atIndex: 0];

        [RenderEncoder setVertexBytes: &ViewportSize
                               length: sizeof(ViewportSize)
                              atIndex: 1];

        [RenderEncoder drawPrimitives: MTLPrimitiveTypeTriangle
                          vertexStart: 0
                          vertexCount: NumberOfColorVertices];

        TextureCommandBuffer = RenderCommandsPtr->TextureVertexCommandBuffers[RenderCommandsPtr->FrameIndex];
        NSUInteger NumberOfTextureVertices = (NSUInteger)TextureCommandBuffer.NumberOfTextureVertices;
        
        [RenderEncoder setRenderPipelineState: [self TexturePipelineState]];

        [RenderEncoder setVertexBuffer: [[self TextureVertexBuffers] objectAtIndex: _currentFrameIndex]
                                offset: 0
                               atIndex: 0];

        [RenderEncoder setFragmentTexture: [self PreviewBitmapTexture]
                                  atIndex: 0];

        [RenderEncoder drawPrimitives: MTLPrimitiveTypeTriangle
                          vertexStart: 0
                          vertexCount: NumberOfTextureVertices];

        [RenderEncoder endEncoding];

        // Schedule a present once the framebuffer is complete using the current drawable
        id<CAMetalDrawable> NextDrawable = [view currentDrawable];
        [CommandBuffer presentDrawable: NextDrawable];

        __block dispatch_semaphore_t semaphore = _frameBoundarySemaphore;
        [CommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
            dispatch_semaphore_signal(semaphore);
        }];

        [CommandBuffer commit];
    }
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size
{

}

@end

global_variable MetalViewDelegate *ViewDelegate;

void PlatformUpdatePreviewTextureSize(uint32 Width, uint32 Height)
{
    MTLTextureDescriptor *TextureDescriptor = [[MTLTextureDescriptor alloc] init];
    TextureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
    TextureDescriptor.width = Width;
    TextureDescriptor.height = Height;
    TextureDescriptor.usage = MTLTextureUsageShaderRead;

    id<MTLTexture> PreviewBitmapTexture = [MetalKitView.device newTextureWithDescriptor: TextureDescriptor];
    ViewDelegate.PreviewBitmapTexture = PreviewBitmapTexture;
}

internal 
void ControllerInput(void *context, IOReturn result, 
                     void *sender, IOHIDValueRef value)
{
    if(result != kIOReturnSuccess) {
        return;
    }

    mac_game_controller *MacGameController = (mac_game_controller *)context;
    
    IOHIDElementRef Element = IOHIDValueGetElement(value);    
    uint32 UsagePage = IOHIDElementGetUsagePage(Element);
    uint32 Usage = IOHIDElementGetUsage(Element);

    //Buttons
    if(UsagePage == kHIDPage_Button) {

        bool32 ButtonState = (bool32)IOHIDValueGetIntegerValue(value);

        if (Usage == MacGameController->ButtonAUsageID) 
        { 
            MacGameController->ButtonAState = ButtonState; 
        } else if (Usage == MacGameController->ButtonBUsageID)
        { 
            MacGameController->ButtonBState = ButtonState; 
        } else if(Usage == MacGameController->ButtonXUsageID) 
        { 
            MacGameController->ButtonXState = ButtonState; 
        } else if(Usage == MacGameController->ButtonYUsageID) 
        { 
            MacGameController->ButtonYState = ButtonState; 
        } else if(Usage == MacGameController->ButtonLeftShoulderUsageID) 
        { 
            MacGameController->ButtonLeftShoulderState = ButtonState; 
        } 
        else if (Usage == MacGameController->ButtonLeftShoulder2UsageID)
        {
            MacGameController->ButtonLeftShoulder2State = ButtonState; 
        }
        else if(Usage == MacGameController->ButtonRightShoulderUsageID) 
        { 
            MacGameController->ButtonRightShoulderState = ButtonState; 
        } 
        else if (Usage == MacGameController->ButtonRightShoulder2UsageID)
        {
            MacGameController->ButtonRightShoulder2State = ButtonState; 
        }
        else if (Usage == MacGameController->ButtonStartUsageID)
        {
            MacGameController->ButtonStartState = ButtonState;
        } else if (Usage == MacGameController ->ButtonSelectUsageID)
        {
            MacGameController->ButtonSelectState = ButtonState;
        }
    }

    //dPad
    else if(UsagePage == kHIDPage_GenericDesktop) {

        double_t Analog = IOHIDValueGetScaledValue(value, kIOHIDValueScaleTypeCalibrated);
        
        if (Usage == MacGameController->LeftThumbXUsageID) {
            MacGameController->LeftThumbstickX = (real32)Analog;
        }

        if (Usage == MacGameController->LeftThumbYUsageID) {
            MacGameController->LeftThumbstickY = (real32)Analog;
        }

        if(Usage == kHIDUsage_GD_Hatswitch) { 
            int DPadState = (int)IOHIDValueGetIntegerValue(value);
            int32 DPadX = 0;
            int32 DPadY = 0;

            switch(DPadState) {
                case 0: DPadX = 0; DPadY = 1; break;
                case 1: DPadX = 1; DPadY = 1; break;
                case 2: DPadX = 1; DPadY = 0; break;
                case 3: DPadX = 1; DPadY = -1; break;
                case 4: DPadX = 0; DPadY = -1; break;
                case 5: DPadX = -1; DPadY = -1; break;
                case 6: DPadX = -1; DPadY = 0; break;
                case 7: DPadX = -1; DPadY = 1; break;
                default: DPadX = 0; DPadY = 0; break;
            }

            MacGameController->DPadX = DPadX;
            MacGameController->DPadY = DPadY;
        }
    }
}

internal 
void ControllerConnected(void *context, IOReturn result, 
                         void *sender, IOHIDDeviceRef device)
{

    if(result != kIOReturnSuccess) {
        return;
    }

    NSUInteger vendorID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, 
                                                                       CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
    NSUInteger productID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, 
                                                                        CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];

    mac_game_controller *MacGameController = (mac_game_controller *)context;

    if(vendorID == 0x054C && productID == 0x5C4) {
        NSLog(@"Sony Dualshock 4 detected.");

        //  Left Thumb Stick       
        MacGameController->LeftThumbXUsageID = kHIDUsage_GD_X;
        MacGameController->LeftThumbYUsageID = kHIDUsage_GD_Y;
        MacGameController->UsesHatSwitch = true;
 
        MacGameController->ButtonAUsageID = 0x02;
        MacGameController->ButtonBUsageID = 0x03;
        MacGameController->ButtonXUsageID = 0x01;
        MacGameController->ButtonYUsageID = 0x04;

        MacGameController->ButtonLeftShoulderUsageID = 0x05;
        MacGameController->ButtonRightShoulderUsageID = 0x06;

        MacGameController->ButtonLeftShoulder2UsageID = 0x07;
        MacGameController->ButtonRightShoulder2UsageID = 0x08;

        MacGameController->ButtonStartUsageID = 0x0a;
        MacGameController->ButtonSelectUsageID = 0x09;
    }
    
    MacGameController->LeftThumbstickX = 128.0f;
    MacGameController->LeftThumbstickY = 128.0f;

    IOHIDDeviceRegisterInputValueCallback(device, ControllerInput, (void *)MacGameController);  
    IOHIDDeviceSetInputValueMatchingMultiple(device, (__bridge CFArrayRef)@[
        @{@(kIOHIDElementUsagePageKey): @(kHIDPage_GenericDesktop)},
        @{@(kIOHIDElementUsagePageKey): @(kHIDPage_Button)},
    ]);
}

internal
void MacInitGameControllers(mac_game_controller *MacGameController) {
    IOHIDManagerRef HIDManager = IOHIDManagerCreate(kCFAllocatorDefault, 0);

    if (IOHIDManagerOpen(HIDManager, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
        NSLog(@"Error Initializing OSX Handmade Controllers");
        return;
    }

    IOHIDManagerRegisterDeviceMatchingCallback(HIDManager, ControllerConnected, (void *)MacGameController);

    IOHIDManagerSetDeviceMatchingMultiple(HIDManager, (__bridge CFArrayRef)@[
        @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_GamePad)},
        @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_MultiAxisController)},
    ]);
  
	IOHIDManagerScheduleWithRunLoop(HIDManager, 
                                    CFRunLoopGetMain(), 
                                    kCFRunLoopDefaultMode);
}


internal real32
MacGetSecondsElapsed(mach_timebase_info_data_t *TimeBase, uint64 Start, uint64 End)
{
	uint64 Elapsed = (End - Start);
    real32 Result = (real32)(Elapsed * (TimeBase->numer / TimeBase->denom)) / 1000.f / 1000.f / 1000.f;
    return(Result);
}

internal void
SetupAlphaBlendForRenderPipelineColorAttachment(MTLRenderPipelineColorAttachmentDescriptor *ColorRenderBufferAttachment)
{
    ColorRenderBufferAttachment.blendingEnabled = YES;
    ColorRenderBufferAttachment.rgbBlendOperation = MTLBlendOperationAdd;
    ColorRenderBufferAttachment.alphaBlendOperation = MTLBlendOperationAdd;
    ColorRenderBufferAttachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    ColorRenderBufferAttachment.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    ColorRenderBufferAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    ColorRenderBufferAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
}

int main(int argc, const char * argv[]) 
{
    MainWindowDelegate *WindowDelegate = [[MainWindowDelegate alloc] init];

    NSRect ScreenRect = [[NSScreen mainScreen] frame];

#if LAPTOP
    float GlobalRenderWidth = 750;
    float GlobalRenderHeight = 750;
#else
    float GlobalRenderWidth = 1024;
    float GlobalRenderHeight = 1024;
#endif

    NSRect InitialFrame = NSMakeRect((ScreenRect.size.width - GlobalRenderWidth) * 0.5,
                                     (ScreenRect.size.height - GlobalRenderHeight) * 0.5,
                                     GlobalRenderWidth, GlobalRenderHeight);
  
    TextureEditorWindow  *Window = [[TextureEditorWindow alloc] 
                                        initWithContentRect: InitialFrame
                                                  styleMask: NSWindowStyleMaskTitled |
                                                             NSWindowStyleMaskClosable
                                                    backing: NSBackingStoreBuffered
                                                      defer: NO];    

    [Window setBackgroundColor: NSColor.blackColor];
    [Window setTitle: @"Texture Editor"];

    [Window makeKeyAndOrderFront: nil];
    [Window setDelegate: WindowDelegate];
    Window.contentView.layerContentsPlacement = NSViewLayerContentsPlacementCenter;
    Window.contentView.layer.contentsGravity = kCAGravityCenter;
    Window.contentView.wantsLayer = YES;

    MetalKitView = [[MTKView alloc] init];
    MetalKitView.frame = CGRectMake(0, 0, 
                                    GlobalRenderWidth, GlobalRenderHeight); 

    MetalKitView.device = MTLCreateSystemDefaultDevice(); 
    MetalKitView.framebufferOnly = false;
    MetalKitView.layer.contentsGravity = kCAGravityCenter;
    MetalKitView.preferredFramesPerSecond = 60;

    [Window setContentView: MetalKitView];

    ViewDelegate = [[MetalViewDelegate alloc] init];

    texture_editor_render_commands RenderCommands = {}; 
    RenderCommands.ViewportWidth = (int)GlobalRenderWidth;
    RenderCommands.ViewportHeight = (int)GlobalRenderHeight;
    RenderCommands.PreviewTextureWidth = 24;
    RenderCommands.PreviewTextureHeight = 24;
    RenderCommands.UniqueTextureCount = 1;

    texture_editor_texture_buffer TextureBuffer = {};
    TextureBuffer.NumberOfTextures = RenderCommands.UniqueTextureCount;
    TextureBuffer.TexturesLoaded = 0;


    MTLTextureDescriptor *TextureDescriptor = [[MTLTextureDescriptor alloc] init];
    TextureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
    TextureDescriptor.width = RenderCommands.PreviewTextureWidth;
    TextureDescriptor.height = RenderCommands.PreviewTextureHeight;
    TextureDescriptor.usage = MTLTextureUsageShaderRead;

    id<MTLTexture> PreviewBitmapTexture = [MetalKitView.device newTextureWithDescriptor: TextureDescriptor];

    uint32 PageSize = getpagesize();
    uint32 ColorVertexBufferSize = PageSize*1000;
    uint32 TextureVertexBufferSize = PageSize*1000;

    NSMutableArray *ColorVertexBuffers = [[NSMutableArray alloc] init];
    NSMutableArray *TextureVertexBuffers = [[NSMutableArray alloc] init];

    for(uint32 FrameIndex = 0; FrameIndex < 3; FrameIndex++)
    {
        color_vertex_command_buffer ColorVertexCommandBuffer = {};
        ColorVertexCommandBuffer.NumberOfColorVertices = 0;

        ColorVertexCommandBuffer.ColorVertices = (texture_editor_color_vertex *) mmap(0, ColorVertexBufferSize,
                                                                                      PROT_READ | PROT_WRITE,
                                                                                      MAP_PRIVATE | MAP_ANON, -1, 0);

        RenderCommands.ColorVertexCommandBuffers[FrameIndex] = ColorVertexCommandBuffer;

        id<MTLBuffer> ColorVertexBuffer = [MetalKitView.device newBufferWithBytesNoCopy: ColorVertexCommandBuffer.ColorVertices
                                                                                 length: ColorVertexBufferSize
                                                                                options: MTLResourceStorageModeShared
                                                                            deallocator: nil];

        [ColorVertexBuffers addObject: ColorVertexBuffer];

        texture_vertex_command_buffer TextureVertexCommandBuffer = {};
        TextureVertexCommandBuffer.NumberOfTextureVertices = 0;

        TextureVertexCommandBuffer.TextureVertices = (texture_editor_texture_vertex *) mmap(0, TextureVertexBufferSize,
                                                                                            PROT_READ | PROT_WRITE,
                                                                                            MAP_PRIVATE | MAP_ANON, -1, 0);

        RenderCommands.TextureVertexCommandBuffers[FrameIndex] = TextureVertexCommandBuffer;

        id<MTLBuffer> TextureVertexBuffer = [MetalKitView.device newBufferWithBytesNoCopy: TextureVertexCommandBuffer.TextureVertices
                                                                                   length: TextureVertexBufferSize
                                                                                  options: MTLResourceStorageModeShared
                                                                              deallocator: nil];

        [TextureVertexBuffers addObject: TextureVertexBuffer]; 
    }

    ViewDelegate.RenderCommands = RenderCommands; 

    NSString *LibraryFile = [[NSBundle mainBundle] pathForResource: @"ColorShaders" ofType: @"metallib"];
    id<MTLLibrary> ShaderLibrary = [MetalKitView.device newLibraryWithFile: LibraryFile error: nil];
    id<MTLFunction> VertexFunction = [ShaderLibrary newFunctionWithName:@"vertexShader"];
    id<MTLFunction> FragmentFunction = [ShaderLibrary newFunctionWithName:@"fragmentShader"];

    MTLRenderPipelineDescriptor *ColorPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    ColorPipelineStateDescriptor.label = @"2D Game Color Vertices";
    ColorPipelineStateDescriptor.vertexFunction = VertexFunction;
    ColorPipelineStateDescriptor.fragmentFunction = FragmentFunction;
    MTLRenderPipelineColorAttachmentDescriptor *ColorRenderBufferAttachment = ColorPipelineStateDescriptor.colorAttachments[0];
    ColorRenderBufferAttachment.pixelFormat = MetalKitView.colorPixelFormat;
    SetupAlphaBlendForRenderPipelineColorAttachment(ColorRenderBufferAttachment);

    NSError *error = NULL;
    id<MTLRenderPipelineState> ColorPipelineState  = [MetalKitView.device 
                                                        newRenderPipelineStateWithDescriptor:ColorPipelineStateDescriptor
                                                                                       error:&error];

    NSString *TextureShaderLibraryFile = [[NSBundle mainBundle] pathForResource: @"TextureShader" ofType: @"metallib"];
    id<MTLLibrary> TextureShaderLibrary = [MetalKitView.device newLibraryWithFile: TextureShaderLibraryFile error: nil];
    id<MTLFunction> TextureShaderVertexFunction = [TextureShaderLibrary newFunctionWithName:@"vertexShader"];
    id<MTLFunction> TextureShaderFragmentFunction = [TextureShaderLibrary newFunctionWithName:@"fragmentShader"];

    MTLRenderPipelineDescriptor *TexturePipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    TexturePipelineDescriptor.label = @"2D Texture Vertices";
    TexturePipelineDescriptor.vertexFunction = TextureShaderVertexFunction;
    TexturePipelineDescriptor.fragmentFunction = TextureShaderFragmentFunction; 
    MTLRenderPipelineColorAttachmentDescriptor *TextureRenderBufferAttachment = TexturePipelineDescriptor.colorAttachments[0];
    TextureRenderBufferAttachment.pixelFormat = MetalKitView.colorPixelFormat;
    SetupAlphaBlendForRenderPipelineColorAttachment(TextureRenderBufferAttachment);

    id<MTLRenderPipelineState> TexturePipelineState = [MetalKitView.device 
                                                        newRenderPipelineStateWithDescriptor: TexturePipelineDescriptor
                                                                                       error: &error];

    if (error != nil)
    {
        NSLog(@"Error creating texture pipeline");
    }


    id<MTLCommandQueue> CommandQueue = [MetalKitView.device newCommandQueue]; 

    mac_state MacState = {};
    mac_app_path Path = {};
    MacState.Path = &Path;
    MacBuildAppFilePath(MacState.Path);

    [ViewDelegate setMacStatePtr: &MacState];

    texture_editor_keyboard_input KeyboardInput = {};
    [Window setKeyboardInputPtr: &KeyboardInput];
    [ViewDelegate setKeyboardInputPtr: &KeyboardInput];

    mac_game_controller MacGameController = {}; 
    MacInitGameControllers(&MacGameController); 
    [ViewDelegate setGameControllerPtr: &MacGameController];

    texture_editor_input Input[2] = {};
    texture_editor_input *NewInput = &Input[0];
    texture_editor_input *OldInput = &Input[1];

    [ViewDelegate setNewInputPtr: NewInput];
    [ViewDelegate setOldInputPtr: OldInput];

    texture_editor_memory EditorMemory = {};
    EditorMemory.PermanentStorageSize = Megabytes(64);

    void* BaseAddress = 0;
    uint32 AllocationFlags = MAP_PRIVATE | MAP_ANON;

    EditorMemory.PermanentStorage = mmap(BaseAddress,
                                    EditorMemory.PermanentStorageSize,
                                    PROT_READ | PROT_WRITE,
                                    AllocationFlags, -1, 0); 

    if (EditorMemory.PermanentStorage == MAP_FAILED) 
    {
		printf("mmap error: %d  %s", errno, strerror(errno));
        [NSException raise: @"Memory Not Allocated"
                     format: @"Failed to allocate permanent storage"];
    }

    LoadTextures(&EditorMemory, &TextureBuffer);

    ViewDelegate.EditorMemory = EditorMemory; 
    ViewDelegate.CommandQueue = CommandQueue;
    ViewDelegate.ColorPipelineState = ColorPipelineState;
    ViewDelegate.TexturePipelineState = TexturePipelineState;
    ViewDelegate.ColorVertexBuffers = ColorVertexBuffers;
    ViewDelegate.TextureVertexBuffers = TextureVertexBuffers;
    ViewDelegate.PreviewBitmapTexture = PreviewBitmapTexture;
    [ViewDelegate configureMetal];

    [MetalKitView setDelegate: ViewDelegate];

    return NSApplicationMain(argc, argv);
}
