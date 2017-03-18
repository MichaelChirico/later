#include "later.h"
#include <Rcpp.h>

void ensureInitialized();
void doExecLater(Rcpp::Function callback);

// This is just quote(base::sys.nframe()). We create this from R and
// store it, because I don't want to learn how to parse strings into
// call SEXPRs from C/C++.
static SEXP nframes;

// Save a call expression as NFramesCallback.
// [[Rcpp::export]]
void saveNframesCallback(SEXP exp) {
  R_PreserveObject(exp);
  nframes = exp;
}

// Returns true if sys.nframes() returns 0.
bool at_top_level() {
  int frames = Rcpp::as<int>(Rf_eval(nframes, R_GlobalEnv));
  return frames == 0;
}

// [[Rcpp::export]]
void execLater(Rcpp::Function callback) {
  ensureInitialized();
  
  doExecLater(callback);
}