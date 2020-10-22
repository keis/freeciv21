/*####################################################################
###      ***                                     ***               ###
###     *****          ⒻⓇⒺⒺⒸⒾⓋ ②①         *****              ###
###      ***                                     ***               ###
#####################################################################*/

#ifndef FC__NOTIFYREPORT_H
#define FC__NOTIFYREPORT_H

#include "dialogs_g.h"

// Qt
#include <QDialog>
#include <QMessageBox>
#include <QVariant>

// gui-qt
#include "fonts.h"
#include "hudwidget.h"
#include "mapview.h"

class QComboBox;
class QGridLayout;
class QGroupBox;
class QItemSelection;
class QRadioButton;
class QTableView;
class QTableWidget;
class QTextEdit;
class QWidget;

void restart_notify_reports();

/***************************************************************************
 Widget around map view to display informations like demographics report,
 top 5 cities, traveler's report.
***************************************************************************/
class notify_dialog : public fcwidget {
  Q_OBJECT
public:
  notify_dialog(const char *caption, const char *headline, const char *lines,
                QWidget *parent = 0);
  virtual void update_menu();
  ~notify_dialog();
  void restart();

protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

private:
  void paintEvent(QPaintEvent *paint_event);
  void calc_size(int &x, int &y);
  close_widget *cw;
  QLabel *label;
  QVBoxLayout *layout;
  QString qcaption;
  QString qheadline;
  QStringList qlist;
  QFont small_font;
  QPoint cursor;
};

#endif  /* FC__NOTIFYREPORT_H */