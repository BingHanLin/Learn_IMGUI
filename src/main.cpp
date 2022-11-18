#include "mainWindow.hpp"

int main(int, char **) {
  mainWindow app;
  app.init();
  app.run();
  app.cleanUp();
  return 0;
}
