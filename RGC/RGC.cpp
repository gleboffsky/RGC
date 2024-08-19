#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include <string>
HINSTANCE hInst;
HWND hwndMain;
HWND hwndButtonPanel;
RECT clientRect;
int* localPositionPtr = nullptr; 
int figurePosition = 0;
int figureWidthTop = 100;
int figureWidthBottom = 200;
int figureHeight = 200;
int figureScale = 100;
int figureX = 20;
int figureY = 20;
int arcRadius = 50;
int ellipseRadius = 60;
int* positionPtr = &figurePosition;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ScaleDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PositionDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CreateButtons(HWND hwnd);
void UpdateFigurePosition(int position);
void UpdateFigureScale(int scale);
void DrawTrapezoid(HDC hdc, int rectLeft, int rectTop, int rectRight, int rectBottom);
HBRUSH hPanelBackgroundBrush = CreateSolidBrush(RGB(255, 255, 255));

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, GetModuleHandle(0), LoadIcon(0, IDI_APPLICATION),
                     LoadCursor(0, IDC_ARROW), hPanelBackgroundBrush, 0, L"MainWindowClass", LoadIcon(0, IDI_APPLICATION) };
    RegisterClassEx(&wc);
    hwndMain = CreateWindow(wc.lpszClassName, L"Программа с трапецией", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, 0, 0, hInstance, 0);

    hwndButtonPanel = CreateWindow(L"STATIC", 0, WS_CHILD | WS_VISIBLE, 0, clientRect.bottom, clientRect.right, 50, hwndMain, 0, hInstance, 0);
    CreateButtons(hwndButtonPanel);

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Освобождаем ресурсы кисти
    DeleteObject(hPanelBackgroundBrush);
    return msg.wParam;
}

INT_PTR CALLBACK ScaleDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static int* scalePtr;

    switch (uMsg) {
    case WM_INITDIALOG:
        scalePtr = (int*)lParam;
        SetDlgItemInt(hwndDlg, IDC_SCALE_EDIT, *scalePtr, FALSE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        {
            BOOL bTranslated;
            int newScale = GetDlgItemInt(hwndDlg, IDC_SCALE_EDIT, &bTranslated, FALSE);

            if (bTranslated) {
                if (newScale >= 50 && newScale <= 100) {
                    *scalePtr = newScale; // Обновляем значение масштаба
                    UpdateFigureScale(newScale); // Обновляем масштаб фигуры
                    EndDialog(hwndDlg, IDOK);
                }
                else {
                    MessageBox(hwndDlg, L"Масштаб должен быть в диапазоне от 50% до 100%.", L"Ошибка", MB_ICONERROR);
                }
            }
            else {
                MessageBox(hwndDlg, L"Пожалуйста, введите корректное значение масштаба.", L"Ошибка", MB_ICONERROR);
            }
            break;
        }
        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        hInst = ((LPCREATESTRUCT)lParam)->hInstance;
        CreateButtons(hwnd);
        break;
    case WM_SIZE:
    {
        GetClientRect(hwnd, &clientRect);
        // Получите текущие размеры окна с кнопками
        RECT buttonPanelRect;
        GetWindowRect(hwndButtonPanel, &buttonPanelRect);
        int buttonPanelWidth = buttonPanelRect.right - buttonPanelRect.left;
        int buttonPanelHeight = buttonPanelRect.bottom - buttonPanelRect.top;
        // Пересчитайте позицию и переместите окно с кнопками внизу под clientRect
        int newButtonPanelX = 0; 
        int newButtonPanelY = clientRect.bottom;

        MoveWindow(hwndButtonPanel, newButtonPanelX, newButtonPanelY, buttonPanelWidth, buttonPanelHeight, TRUE);
        InvalidateRect(hwnd, 0, TRUE);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        int leftMargin = 20;
        int rightMargin = 20;
        int topMargin = 20;
        int bottomMargin = 20;

        int rectLeft = leftMargin;
        int rectTop = topMargin;
        int rectRight = clientRect.right - rightMargin;
        int rectBottom = clientRect.bottom - bottomMargin;

        // Отрисовка прямоугольной области
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, hBrush);
        Rectangle(hdc, rectLeft, rectTop, rectRight, rectBottom);
        DeleteObject(hBrush);

        int widthTop = (rectRight - rectLeft - 40) * figureScale / 100;
        int widthBottom = (rectRight - rectLeft - 40) * (figureScale * 4 / 5) / 100;
        int height = (rectBottom - rectTop - 40) * figureScale / 100;

        // Обновляем параметры трапеции
        //figureX = rectLeft + (rectRight - rectLeft - widthTop) / 2;
        //figureY = rectTop + (rectBottom - rectTop - height) / 2;
        figureWidthTop = widthTop;
        figureWidthBottom = widthBottom;
        figureHeight = height;

        DrawTrapezoid(hdc, rectLeft, rectTop, rectRight, rectBottom);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDD_MY_SCALE_DIALOG) {
            int newScale = 0; // Обновляем масштаб (необходимо реализовать)
            INT_PTR result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MY_SCALE_DIALOG), hwnd, ScaleDialogProc, (LPARAM)&newScale);
            if (result == IDOK) {
                // Обновляем масштаб (необходимо реализовать)
                InvalidateRect(hwnd, 0, TRUE);
            }
            else if (result == -1) {
                MessageBox(hwnd, L"Ошибка открытия диалогового окна.", L"Ошибка", MB_ICONERROR);
            }
        }
        else if (LOWORD(wParam) == IDD_MY_POSITION_DIALOG) {
            // Открываем диалоговое окно для выбора позиции
            int newPosition = figurePosition;
            INT_PTR result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MY_POSITION_DIALOG), hwnd, PositionDialogProc, (LPARAM)&figurePosition);
            
            ;
            if (result == IDOK) {
                figurePosition = newPosition;
                InvalidateRect(hwnd, 0, TRUE);
            }
            else if (result == -1) {
                MessageBox(hwnd, L"Ошибка открытия диалогового окна.", L"Ошибка", MB_ICONERROR);
            }
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CreateButtons(HWND hwnd) {

    CreateWindow(L"BUTTON", L"Задать масштаб", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 10, 150, 30, hwnd, (HMENU)IDD_MY_SCALE_DIALOG, hInst, 0);
    CreateWindow(L"BUTTON", L"Задать расположение", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 200, 10, 150, 30, hwnd, (HMENU)IDD_MY_POSITION_DIALOG, hInst, 0);
}

void UpdateFigurePosition(int position) {
    // Получаем размеры клиентской области
    RECT clientRect;
    GetClientRect(hwndMain, &clientRect);

    int rectLeft = clientRect.left;
    int rectTop = clientRect.top;
    int rectRight = clientRect.right;
    int rectBottom = clientRect.bottom;

    switch (position) {
    case 0: // Верх-лево
        figureX = rectLeft + 20;
        figureY = rectTop + 20;
        break;
    case 1: // Верх-центр
        figureX = (rectLeft + rectRight - figureWidthTop) / 2;
        figureY = rectTop + 20;
        break;
    case 2: // Верх-право
        figureX = rectRight - figureWidthTop - 20;
        figureY = rectTop + 20;
        break;
    case 3: // Центр
        figureX = (rectLeft + rectRight - figureWidthTop) / 2;
        figureY = (rectTop + rectBottom - figureHeight) / 2;
        break;
    case 4: // Низ-лево
        figureX = rectLeft + 20;
        figureY = rectBottom - figureHeight - 20;
        break;
    case 5: // Низ-право
        figureX = rectRight - figureWidthTop - 20;
        figureY = rectBottom - figureHeight - 20;
        break;
    }

    // Проверяем, чтобы фигура не выходила за клиентскую область
    if (figureX < rectLeft) {
        figureX = rectLeft;
    }
    else if (figureX + figureWidthTop > rectRight) {
        figureX = rectRight - figureWidthTop;
    }

    if (figureY < rectTop) {
        figureY = rectTop;
    }
    else if (figureY + figureHeight > rectBottom) {
        figureY = rectBottom - figureHeight;
    }
}

void UpdateFigureScale(int scale) {
    figureScale = scale;
}

void DrawTrapezoid(HDC hdc, int rectLeft, int rectTop, int rectRight, int rectBottom) {

    int widthTop = (rectRight - rectLeft - 40) * figureScale / 100; 
    int widthBottom = (rectRight - rectLeft - 40) * (figureScale * 4 / 5) / 100; 
    int height = (rectBottom - rectTop - 40) * figureScale / 100; 
    int left = figureX; 
    int top = figureY;

    POINT points[4];
    points[0].x = left;
    points[0].y = top;
    points[1].x = left + widthTop;
    points[1].y = top;
    points[2].x = left + widthBottom;
    points[2].y = top + height;
    points[3].x = left + (widthTop - widthBottom);
    points[3].y = top + height;
    Polygon(hdc, points, 4);
}

INT_PTR CALLBACK PositionDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        //localPositionPtr = (int*)lParam; // Преобразуем lParam в указатель на int
        //wchar_t debugMsgg[100]; // Буфер для сообщения
        //swprintf_s(debugMsgg, L"localPositionPtr1: %d", *localPositionPtr);
        //OutputDebugString(debugMsgg);
        CheckRadioButton(hwndDlg, IDC_TOP_LEFT, IDC_BOTTOM_RIGHT, IDC_TOP_LEFT + *localPositionPtr);

        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        {
            if (localPositionPtr) {
                int newPosition = -1;
                if (IsDlgButtonChecked(hwndDlg, IDC_TOP_LEFT)) newPosition = 0;
                else if (IsDlgButtonChecked(hwndDlg, IDC_TOP_CENTER)) newPosition = 1;
                else if (IsDlgButtonChecked(hwndDlg, IDC_TOP_RIGHT)) newPosition = 2;
                else if (IsDlgButtonChecked(hwndDlg, IDC_CENTER)) newPosition = 3;
                else if (IsDlgButtonChecked(hwndDlg, IDC_BOTTOM_LEFT)) newPosition = 4;
                else if (IsDlgButtonChecked(hwndDlg, IDC_BOTTOM_RIGHT)) newPosition = 5;

                //wchar_t debugMsg[100];
                //swprintf_s(debugMsg, sizeof(debugMsg) / sizeof(debugMsg[0]), L"Выбрана позиция: %d\n", newPosition);
                //OutputDebugString(debugMsg);

                *localPositionPtr = newPosition;

                wchar_t debugMsgg[100]; // Буфер для сообщения
                swprintf_s(debugMsgg, L"localPositionPtr2: %d", *localPositionPtr);
                OutputDebugString(debugMsgg);
                UpdateFigurePosition(newPosition);
            }
            
            EndDialog(hwndDlg, IDOK);
            break;
        }
        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
