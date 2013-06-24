#include "frmpreferences.h"
#include "ui_frmpreferences.h"
#include "mainwindow.h"
#include "appwidesettings.h"

using namespace widesettings;
frmpreferences::frmpreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmpreferences)
{
    ui->setupUi(this);

    connect(ui->set_tabbar_show,SIGNAL(toggled(bool)),this,SLOT(_on_toggle_tabbar_hide(bool)));
    connect(ui->set_tabbar_vertical,SIGNAL(toggled(bool)),this,SLOT(_on_toggle_tabbar_vertical(bool)));
    connect(ui->set_tabbar_lock,SIGNAL(toggled(bool)),this,SLOT(_on_toggle_tabbar_lock(bool)));
    connect(ui->set_tabbar_reduce,SIGNAL(toggled(bool)),this,SLOT(_on_toggle_tabbar_reduce(bool)));
    connect(ui->set_tabbar_colouractive,SIGNAL(toggled(bool)),this,SLOT(_on_toggle_tabbar_highlight(bool)));


    //Preset all checkboxes and such
    QSettings* s = MainWindow::instance()->getSettings();
    ui->set_tabbar_show->setChecked(s->value(SETTING_TABBAR_HIDE,false).toBool());
    ui->set_tabbar_lock->setChecked(!s->value(SETTING_TABBAR_MOVABLE,true).toBool());
    ui->set_tabbar_vertical->setChecked(s->value(SETTING_TABBAR_VERTICAL,false).toBool());
    ui->set_tabbar_reduce->setChecked(s->value(SETTING_TABBAR_REDUCE,true).toBool());
    ui->set_tabbar_colouractive->setChecked(s->value(SETTING_TABBAR_HIGHLIGHT,true).toBool());

    //Disable controls that aren't usable yet...
    ui->set_tabbar_closeicons->setEnabled(false);
    ui->set_tabbar_darken->setEnabled(false);
    ui->set_tabbar_multiline->setEnabled(false);
    ui->set_tabbar_doubleclicktoclose->setEnabled(false);
    ui->set_documentlist_show->setEnabled(false);
    ui->set_statusBar_show->setEnabled(false);
    ui->set_menu_hide->setEnabled(false);
    ui->set_toolbar_big->setEnabled(false);
    ui->set_toolbar_hide->setEnabled(false);
    ui->set_toolbar_iconstandard->setEnabled(false);
    ui->set_toolbar_small->setEnabled(false);
    ui->set_localization_language->setEnabled(false);
    setFixedSize(width(),height());
}

frmpreferences::~frmpreferences()
{
    delete ui;
}

void frmpreferences::_on_toggle_tabbar_hide( bool on )
{
    widesettings::apply_tabbar_hide(on);
}

void frmpreferences::_on_toggle_tabbar_vertical( bool on )
{
    widesettings::apply_tabbar_vertical(on);
}


void frmpreferences::_on_toggle_tabbar_lock( bool on )
{
    widesettings::apply_tabbar_movable(!on);
}

void frmpreferences::_on_toggle_tabbar_reduce( bool on )
{
    widesettings::apply_tabbar_reduce( on );
}

void frmpreferences::_on_toggle_tabbar_highlight( bool on )
{
    widesettings::apply_tabbar_highlight( on );
}
