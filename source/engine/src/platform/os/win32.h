#if defined(_WIN32) || defined(_WIN64)

#define import __declspec(dllimport)

extern "C"
{
    #define WS_EX_ACCEPTFILES         0x00000010L
    #define WS_EX_APPWINDOW           0x00040000L
    #define WS_EX_CLIENTEDGE          0x00000200L
    #define WS_EX_COMPOSITED          0x02000000L
    #define WS_EX_CONTEXTHELP         0x00000400L
    #define WS_EX_CONTROLPARENT       0x00010000L
    #define WS_EX_DLGMODALFRAME       0x00000001L
    #define WS_EX_LAYERED             0x00080000
    #define WS_EX_LAYOUTRTL           0x00400000L
    #define WS_EX_LEFT                0x00000000L
    #define WS_EX_LEFTSCROLLBAR       0x00004000L
    #define WS_EX_LTRREADING          0x00000000L
    #define WS_EX_MDICHILD            0x00000040L
    #define WS_EX_NOACTIVATE          0x08000000L
    #define WS_EX_NOINHERITLAYOUT     0x00100000L
    #define WS_EX_NOPARENTNOTIFY      0x00000004L
    #define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
    #define WS_EX_RIGHT               0x00001000L
    #define WS_EX_RIGHTSCROLLBAR      0x00000000L
    #define WS_EX_RTLREADING          0x00002000L
    #define WS_EX_STATICEDGE          0x00020000L
    #define WS_EX_TOOLWINDOW          0x00000080L
    #define WS_EX_TOPMOST             0x00000008L
    #define WS_EX_TRANSPARENT         0x00000020L
    #define WS_EX_WINDOWEDGE          0x00000100L
    #define WS_EX_OVERLAPPEDWINDOW (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
    #define WS_EX_PALETTEWINDOW    (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

    #define WS_BORDER       0x00800000L
    #define WS_CAPTION      0x00C00000L
    #define WS_CHILD        0x40000000L
    #define WS_CHILDWINDOW  0x40000000L
    #define WS_CLIPCHILDREN 0x02000000L
    #define WS_CLIPSIBLINGS 0x04000000L
    #define WS_DISABLED     0x08000000L
    #define WS_DLGFRAME     0x00400000L
    #define WS_GROUP        0x00020000L
    #define WS_HSCROLL      0x00100000L
    #define WS_ICONIC       0x20000000L
    #define WS_MAXIMIZE     0x01000000L
    #define WS_MAXIMIZEBOX  0x00010000L
    #define WS_MINIMIZE     0x20000000L
    #define WS_MINIMIZEBOX  0x00020000L
    #define WS_OVERLAPPED   0x00000000L
    #define WS_POPUP        0x80000000L
    #define WS_SIZEBOX      0x00040000L
    #define WS_SYSMENU      0x00080000L
    #define WS_TABSTOP      0x00010000L
    #define WS_THICKFRAME   0x00040000L
    #define WS_TILED        0x00000000L
    #define WS_VISIBLE      0x10000000L
    #define WS_VSCROLL      0x00200000L
    #define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
    #define WS_POPUPWINDOW      (WS_POPUP | WS_BORDER | WS_SYSMENU)
    #define WS_TILEDWINDOW      (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

    #define CS_BYTEALIGNCLIENT 0x1000
    #define CS_BYTEALIGNWINDOW 0x2000
    #define CS_CLASSDC         0x0040
    #define CS_DBLCLKS         0x0008
    #define CS_DROPSHADOW      0x00020000
    #define CS_GLOBALCLASS     0x4000
    #define CS_HREDRAW         0x0002
    #define CS_NOCLOSE         0x0200
    #define CS_OWNDC           0x0020
    #define CS_PARENTDC        0x0080
    #define CS_SAVEBITS        0x0800
    #define CS_VREDRAW         0x0001

    #define PM_NOREMOVE 0x0000
    #define PM_REMOVE   0x0001
    #define PM_NOYIELD  0x0002

    #define CW_USEDEFAULT (int)0x80000000

    #define LPD_SUPPORT_OPENGL 0x00000020

	#define PFD_DRAW_TO_WINDOW 0x00000004
	#define PFD_DOUBLEBUFFER   0x00000001
	#define PFD_TYPE_RGBA      0
	#define PFD_MAIN_PLANE     0

	#define GWLP_USERDATA -21

    #ifdef _WIN64
        typedef int half_ptr;
        typedef __int64 int_ptr;
        typedef __int64 long_ptr;
        typedef unsigned int uhalf_ptr;
        typedef unsigned __int64 uint_ptr;
        typedef unsigned __int64 ulong_ptr;
    #else
        typedef short half_ptr;
        typedef int int_ptr;
        typedef long long_ptr;
        typedef unsigned short uhalf_ptr;
        typedef unsigned int uint_ptr;
        typedef unsigned long ulong_ptr;
    #endif

    typedef long_ptr(__stdcall WindowProc)(void* handle, unsigned int msg, uint_ptr wparam, long_ptr lparam);

    struct WNDCLASSEXA
    {
        unsigned int size;
        unsigned int style;
        WindowProc*  callback;
        int          cls_extra;
        int          wnd_extra;
        void*        instance;
        void*        icon;
        void*        cursor;
        void*        br_background;
        const char*  menu_name;
        const char*  class_name;
        void*        icon_sm;
    };

    struct WNDCLASSEXW
    {
        unsigned int   size;
        unsigned int   style;
        WindowProc*    callback;
        int            cls_extra;
        int            wnd_extra;
        void*          instance;
        void*          icon;
        void*          cursor;
        void*          br_background;
        const wchar_t* menu_name;
        const wchar_t* class_name;
        void*          icon_sm;
    };

    struct POINT
    {
        long x;
        long y;
    };

    struct MSG
    {
        void*         handle;
        unsigned int  msg;
        uint_ptr      wparam;
        long_ptr      lparam;
        unsigned long time;
        POINT         p;
        unsigned long priv;
    };

    struct PIXELFORMATDESCRIPTOR
    {
        unsigned short size;
        unsigned short version;
        unsigned long  flags;
        unsigned char  pixel_type;
        unsigned char  colour_bits;
        unsigned char  red_bits;
        unsigned char  red_shift;
        unsigned char  green_bits;
        unsigned char  green_shift;
        unsigned char  blue_bits;
        unsigned char  blue_shift;
        unsigned char  alpha_bits;
        unsigned char  alpha_shift;
        unsigned char  accum_bits;
        unsigned char  accum_red_bits;
        unsigned char  accum_green_bits;
        unsigned char  accum_blue_bits;
        unsigned char  accum_alpha_bits;
        unsigned char  depth_bits;
        unsigned char  stencil_bits;
        unsigned char  aux_buffers;
        unsigned char  layer_type;
        unsigned char  reserved;
        unsigned long  layer_mask;
        unsigned long  visible_mask;
        unsigned long  damage_mask;
    };

    import int RegisterClassExW(const WNDCLASSEXW *wc);
    import int RegisterClassExA(const WNDCLASSEXA *wc);
    
    import void* CreateWindowExW(
        unsigned long ex_style, const wchar_t* cls_name, const wchar_t* wnd_name, unsigned long style,
        int x, int y, int width, int height, void* wnd_parent, void* menu, void* instance, void* param
    );
    import void* CreateWindowExA(
        unsigned long ex_style, const char* cls_name, const char* wnd_name, unsigned long style,
        int x, int y, int width, int height, void* wnd_parent, void* menu, void* instance, void* param
    );

    import long_ptr __stdcall DefWindowProcW(void* handle, unsigned int msg, uint_ptr wparam, long_ptr lparam);
    import long_ptr __stdcall DefWindowProcA(void* handle, unsigned int msg, uint_ptr wparam, long_ptr lparam);

    import void* GetModuleHandleW(const wchar_t* name);
    import void* GetModuleHandleA(const char* name);

    import int PeekMessageW(MSG* msg, void* handle, unsigned int fmin, unsigned int fmax, unsigned int remove_msg);
    import int PeekMessageA(MSG* msg, void* handle, unsigned int fmin, unsigned int fmax, unsigned int remove_msg);

    import long_ptr DispatchMessageW(const MSG* msg);
    import long_ptr DispatchMessageA(const MSG* msg);

    import void* LoadLibraryW(const wchar_t* name);
    import void* LoadLibraryA(const char* name);

    import int TranslateMessage(const MSG* msg);
    
    import int DestroyWindow(void* handle);

    import unsigned long GetLastError();

    import void* GetDC(void* handle);

    import int ReleaseDC(void* handle, void* dc);

    import int SwapBuffers(void* dc);

    import void* wglGetProcAddress(const char* name);

    import int DescribePixelFormat(void* dc, int format, unsigned int bytes, PIXELFORMATDESCRIPTOR* pfd);

    import int ChoosePixelFormat(void* dc, const PIXELFORMATDESCRIPTOR* pfd);

    import int SetPixelFormat(void* dc, int format, const PIXELFORMATDESCRIPTOR* pfd);

    import void* wglCreateContext(void* dc);

    import int wglMakeCurrent(void* dc, void* hglrc);

    import int wglDeleteContext(void* hglrc);

    import int FreeLibrary(void* mod);

    #ifdef UNICODE_ON
        #define WNDCLASSEX WNDCLASSEXW
        #define RegisterClassEx RegisterClassExW
        #define CreateWindowEx CreateWindowExW
        #define DefWindowProc DefWindowProcW
        #define GetModuleHandle GetModuleHandleW
        #define PeekMessage PeekMessageW
        #define DispatchMessage DispatchMessageW
        #define LoadLibrary LoadLibraryW
    #else 
        #define WNDCLASSEX WNDCLASSEXA
        #define RegisterClassEx RegisterClassExA
        #define CreateWindowEx CreateWindowExA
        #define DefWindowProc DefWindowProcA
        #define GetModuleHandle GetModuleHandleA
        #define PeekMessage PeekMessageA
        #define DispatchMessage DispatchMessageA
        #define LoadLibrary LoadLibraryA
    #endif
}

#endif