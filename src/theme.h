#ifndef THEME_H
#define THEME_H

class QApplication;
class QMainWindow;

namespace Theme {

void applyApplicationTheme(QApplication *app);
void polishMainWindow(QMainWindow *window);

}

#endif // THEME_H
