#ifdef _WIN32

#include <Rcpp.h>
#include <Rinternals.h>
#include <queue>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "later.h"

using namespace Rcpp;

// Whether we have initialized the message-only window.
int initialized = 0;

// The handle to the message-only window
HWND hwnd;

// Pointer to the timer, used to cancel
UINT_PTR pTimer;

// The queue of user-provided callbacks that are scheduled to be
// executed.
std::queue<Rcpp::Function> callbacks;

static bool executeHandlers() {
  if (!at_top_level()) {
    // It's not safe to run arbitrary callbacks when other R code
    // is already running. Wait until we're back at the top level.
    return false;
  }
  
  // TODO: What to do about errors that occur in async handlers?
  while (!callbacks.empty()) {
    Rcpp::Function first = callbacks.front();
    callbacks.pop();
    first();
  }
  return true;
}

LRESULT CALLBACK callbackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_TIMER:
    REprintf(".");
    if (executeHandlers()) {
      KillTimer(hwnd, pTimer);
    }
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

void ensureInitialized() {
  if (!initialized) {
    static const char* class_name = "R_LATER_WINDOW_CLASS";
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = callbackWndProc;
    wc.hInstance = NULL;
    wc.lpszClassName = class_name;
    if (!RegisterClassEx(&wc)) {
      Rf_error("Failed to register window class");
    }

    hwnd = CreateWindowEx(0, class_name, "dummy_name", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (!hwnd) {
      Rf_error("Failed to create message-only window");
    }
    
    initialized = 1;
  }
}

void doExecLater(Rcpp::Function callback) {
  callbacks.push(callback);
  
  pTimer = SetTimer(hwnd, 1, USER_TIMER_MINIMUM, NULL);
}

#endif // ifdef _WIN32