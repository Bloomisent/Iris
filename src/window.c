#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/AST.h"
#include "include/visitor.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HWND g_iris_window = NULL;
static HDC g_iris_backbuffer_dc = NULL;
static HBITMAP g_iris_backbuffer_bitmap = NULL;
static HBITMAP g_iris_backbuffer_old_bitmap = NULL;
static int g_iris_window_width = 0;
static int g_iris_window_height = 0;
static int g_iris_should_close = 0;
static int g_iris_mouse_x = 0;
static int g_iris_mouse_y = 0;
static int g_iris_mouse_down = 0;

static LRESULT CALLBACK Iris_WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY:
            g_iris_should_close = 1;
            PostQuitMessage(0);
            return 0;
        case WM_MOUSEMOVE:
            g_iris_mouse_x = (int)(short)LOWORD(lparam);
            g_iris_mouse_y = (int)(short)HIWORD(lparam);
            return 0;
        case WM_LBUTTONDOWN:
            g_iris_mouse_down = 1;
            return 0;
        case WM_LBUTTONUP:
            g_iris_mouse_down = 0;
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static AST_T* require_number(Visitor_T* visitor, AST_T* arg, const char* fn, int arg_index) {
    AST_T* v = Visitor_Visit(visitor, arg);
    if (v->type != AST_NUMBER) {
        printf("Tripped on function '%s', argument %d expects a number but did not receive one\n", fn, arg_index);
        exit(1);
    }
    return v;
}

AST_T* builtin_function_window_create(Visitor_T* visitor, AST_T** args, int args_size) {
    if (args_size != 3) {
        printf("Tripped on function 'windowCreate', expected 3 arguments (title, width, height), got %d\n", args_size);
        exit(1);
    }
    AST_T* title = Visitor_Visit(visitor, args[0]);
    if (title->type != AST_STRING) {
        printf("Tripped on function 'windowCreate', argument 1 expects a string but did not receive one\n");
        exit(1);
    }
    AST_T* width = require_number(visitor, args[1], "windowCreate", 2);
    AST_T* height = require_number(visitor, args[2], "windowCreate", 3);

    g_iris_window_width = (int)width->number_value;
    g_iris_window_height = (int)height->number_value;

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = Iris_WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "IrisWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    RECT rect;
    rect.left = 0; rect.top = 0;
    rect.right = g_iris_window_width;
    rect.bottom = g_iris_window_height;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    g_iris_window = CreateWindowExA(
        0, "IrisWindowClass", title->string_value,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, wc.hInstance, NULL
    );

    if (!g_iris_window) {
        printf("Tripped on function 'windowCreate', CreateWindowExA failed\n");
        exit(1);
    }

    ShowWindow(g_iris_window, SW_SHOW);
    UpdateWindow(g_iris_window);

    HDC screen_dc = GetDC(g_iris_window);
    g_iris_backbuffer_dc = CreateCompatibleDC(screen_dc);
    g_iris_backbuffer_bitmap = CreateCompatibleBitmap(screen_dc, g_iris_window_width, g_iris_window_height);
    g_iris_backbuffer_old_bitmap = (HBITMAP)SelectObject(g_iris_backbuffer_dc, g_iris_backbuffer_bitmap);
    ReleaseDC(g_iris_window, screen_dc);

    g_iris_should_close = 0;
    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_window_should_close(Visitor_T* visitor, AST_T** args, int args_size) {
    if (args_size != 0) {
        printf("Tripped on function 'windowShouldClose', expected 0 arguments, got %d\n", args_size);
        exit(1);
    }

    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    AST_T* result = Init_AST(AST_NUMBER);
    result->number_value = g_iris_should_close ? 1.0f : 0.0f;
    return result;
};

AST_T* builtin_function_window_clear(Visitor_T* visitor, AST_T** args, int args_size) {
    if (args_size != 3) {
        printf("Tripped on function 'windowClear', expected 3 arguments (r, g, b), got %d\n", args_size);
        exit(1);
    }
    AST_T* r = require_number(visitor, args[0], "windowClear", 1);
    AST_T* g = require_number(visitor, args[1], "windowClear", 2);
    AST_T* b = require_number(visitor, args[2], "windowClear", 3);

    RECT full;
    full.left = 0; full.top = 0;
    full.right = g_iris_window_width;
    full.bottom = g_iris_window_height;

    HBRUSH brush = CreateSolidBrush(RGB((int)r->number_value, (int)g->number_value, (int)b->number_value));
    FillRect(g_iris_backbuffer_dc, &full, brush);
    DeleteObject(brush);

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_window_draw_rect(Visitor_T* visitor, AST_T** args, int args_size) {
    if (args_size != 7) {
        printf("Tripped on function 'windowDrawRect', expected 7 arguments (x, y, w, h, r, g, b), got %d\n", args_size);
        exit(1);
    }
    AST_T* x = require_number(visitor, args[0], "windowDrawRect", 1);
    AST_T* y = require_number(visitor, args[1], "windowDrawRect", 2);
    AST_T* w = require_number(visitor, args[2], "windowDrawRect", 3);
    AST_T* h = require_number(visitor, args[3], "windowDrawRect", 4);
    AST_T* r = require_number(visitor, args[4], "windowDrawRect", 5);
    AST_T* g = require_number(visitor, args[5], "windowDrawRect", 6);
    AST_T* b = require_number(visitor, args[6], "windowDrawRect", 7);

    RECT rect;
    rect.left = (int)x->number_value;
    rect.top = (int)y->number_value;
    rect.right = (int)(x->number_value + w->number_value);
    rect.bottom = (int)(y->number_value + h->number_value);

    HBRUSH brush = CreateSolidBrush(RGB((int)r->number_value, (int)g->number_value, (int)b->number_value));
    FillRect(g_iris_backbuffer_dc, &rect, brush);
    DeleteObject(brush);

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_window_draw_text(Visitor_T* visitor, AST_T** args, int args_size) {
    if (args_size != 5) {
        printf("Tripped on function 'windowDrawText', expected 5 arguments (x, y, text, r, g, b), got %d\n", args_size);
        exit(1);
    }
    AST_T* x = require_number(visitor, args[0], "windowDrawText", 1);
    AST_T* y = require_number(visitor, args[1], "windowDrawText", 2);
    AST_T* text = Visitor_Visit(visitor, args[2]);
    if (text->type != AST_STRING) {
        printf("Tripped on function 'windowDrawText', argument 3 expects a string but did not receive one\n");
        exit(1);
    }
    AST_T* r = require_number(visitor, args[3], "windowDrawText", 4);
    AST_T* g = require_number(visitor, args[4], "windowDrawText", 5);

    SetTextColor(g_iris_backbuffer_dc, RGB((int)r->number_value, (int)g->number_value, 0));
    SetBkMode(g_iris_backbuffer_dc, TRANSPARENT);
    TextOutA(g_iris_backbuffer_dc, (int)x->number_value, (int)y->number_value, text->string_value, (int)strlen(text->string_value));

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_window_present(Visitor_T* visitor, AST_T** args, int args_size) {
    if (args_size != 0) {
        printf("Tripped on function 'windowPresent', expected 0 arguments, got %d\n", args_size);
        exit(1);
    }
    HDC screen_dc = GetDC(g_iris_window);
    BitBlt(screen_dc, 0, 0, g_iris_window_width, g_iris_window_height, g_iris_backbuffer_dc, 0, 0, SRCCOPY);
    ReleaseDC(g_iris_window, screen_dc);
    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_window_mouse_x(Visitor_T* visitor, AST_T** args, int args_size) {
    AST_T* result = Init_AST(AST_NUMBER);
    result->number_value = (float)g_iris_mouse_x;
    return result;
};

AST_T* builtin_function_window_mouse_y(Visitor_T* visitor, AST_T** args, int args_size) {
    AST_T* result = Init_AST(AST_NUMBER);
    result->number_value = (float)g_iris_mouse_y;
    return result;
};

AST_T* builtin_function_window_mouse_pressed(Visitor_T* visitor, AST_T** args, int args_size) {
    AST_T* result = Init_AST(AST_NUMBER);
    result->number_value = g_iris_mouse_down ? 1.0f : 0.0f;
    return result;
};

AST_T* builtin_function_window_close(Visitor_T* visitor, AST_T** args, int args_size) {
    if (g_iris_backbuffer_dc) {
        SelectObject(g_iris_backbuffer_dc, g_iris_backbuffer_old_bitmap);
        DeleteObject(g_iris_backbuffer_bitmap);
        DeleteDC(g_iris_backbuffer_dc);
        g_iris_backbuffer_dc = NULL;
    }
    if (g_iris_window) {
        DestroyWindow(g_iris_window);
        g_iris_window = NULL;
    }
    return Init_AST(AST_NOOP);
};

#else /* not _WIN32 so we can make the interpreter buildable/testable on other platforms */

static AST_T* window_unsupported(const char* fn) {
    printf("Tripped on function '%s', windowing is only supported when compiled for Windows (_WIN32)\n", fn);
    exit(1);
    return Init_AST(AST_NOOP);
}

AST_T* builtin_function_window_create(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowCreate"); }
AST_T* builtin_function_window_should_close(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowShouldClose"); }
AST_T* builtin_function_window_clear(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowClear"); }
AST_T* builtin_function_window_draw_rect(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowDrawRect"); }
AST_T* builtin_function_window_draw_text(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowDrawText"); }
AST_T* builtin_function_window_present(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowPresent"); }
AST_T* builtin_function_window_mouse_x(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowMouseX"); }
AST_T* builtin_function_window_mouse_y(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowMouseY"); }
AST_T* builtin_function_window_mouse_pressed(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowMousePressed"); }
AST_T* builtin_function_window_close(Visitor_T* visitor, AST_T** args, int args_size) { return window_unsupported("windowClose"); }

#endif
