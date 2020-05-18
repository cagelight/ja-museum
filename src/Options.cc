#include "Options.hh"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

struct JAPortalOptionsWindow::PrivateData {
	QLineEdit * le_data_dir;
	QLineEdit * le_output_dir;
};

JAPortalOptionsWindow::JAPortalOptionsWindow(QWidget * parent) : QWidget(parent, Qt::Window), m_data { new PrivateData } {
	
	QVBoxLayout * main_lay = new QVBoxLayout { this };
	
	QWidget * main_area_wid = new QWidget { this };
	QGridLayout * main_area_lay = new QGridLayout { main_area_wid };
	main_area_lay->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	main_lay->addWidget(main_area_wid);
	
	QLabel * data_label = new QLabel { "Data Directory:", main_area_wid };
	QLabel * output_label = new QLabel { "Output Directory:", main_area_wid };
	main_area_lay->addWidget(data_label, 0, 0);
	main_area_lay->addWidget(output_label, 1, 0);
	
	m_data->le_data_dir = new QLineEdit { main_area_wid };
	m_data->le_data_dir->setMinimumWidth(300);
	m_data->le_output_dir = new QLineEdit { main_area_wid };
	m_data->le_output_dir->setMinimumWidth(300);
	main_area_lay->addWidget(m_data->le_data_dir, 0, 1);
	main_area_lay->addWidget(m_data->le_output_dir, 1, 1);
	
	QPushButton * data_dir_select = new QPushButton { "Choose...", main_area_wid };
	QPushButton * output_dir_select = new QPushButton { "Choose...", main_area_wid };
	main_area_lay->addWidget(data_dir_select, 0, 2);
	main_area_lay->addWidget(output_dir_select, 1, 2);
	connect(data_dir_select, &QPushButton::clicked, this, [this](){
		QString dir = QFileDialog::getExistingDirectory(this, "Select Data Directory", m_data->le_data_dir->text());
		if (dir.isEmpty()) return;
		m_data->le_data_dir->setText(dir);
	});
	connect(output_dir_select, &QPushButton::clicked, this, [this](){
		QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", m_data->le_output_dir->text());
		if (dir.isEmpty()) return;
		m_data->le_output_dir->setText(dir);
	});
	
	QWidget * main_button_wid = new QWidget { this };
	QHBoxLayout * main_button_lay = new QHBoxLayout { main_button_wid };
	main_button_lay->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	main_lay->addWidget(main_button_wid);
	
	QPushButton * apply_button = new QPushButton { "Apply", main_button_wid };
	QPushButton * close_button = new QPushButton { "Close", main_button_wid };
	main_button_lay->addWidget(apply_button);
	main_button_lay->addWidget(close_button);
	connect(apply_button, &QPushButton::clicked, this, &JAPortalOptionsWindow::apply_gui);
	connect(close_button, &QPushButton::clicked, this, &JAPortalOptionsWindow::hide);
	
	hide();
}

JAPortalOptionsWindow::~JAPortalOptionsWindow() {
	
}

void JAPortalOptionsWindow::showEvent(QShowEvent *) {
	populate_gui();
}

void JAPortalOptionsWindow::load() {
	JAPortalOptions opt;
	QSettings settings { "Cagelight", "JAPortal" };
	
	opt.data_dir = QDir { settings.value("DATA_DIR", QDir::homePath()).toString() };
	opt.output_dir = QDir { settings.value("OUTPUT_DIR", QDir::homePath()).toString() };
	
	apply(opt);
}

void JAPortalOptionsWindow::save() {
	QSettings settings { "Cagelight", "JAPortal" };
	settings.setValue("DATA_DIR", m_opt.data_dir.absolutePath());
	settings.setValue("OUTPUT_DIR", m_opt.output_dir.absolutePath());
}

void JAPortalOptionsWindow::populate_gui() {
	m_data->le_data_dir->setText(m_opt.data_dir.absolutePath());
	m_data->le_output_dir->setText(m_opt.output_dir.absolutePath());
}

void JAPortalOptionsWindow::apply(JAPortalOptions const & opt) {
	m_opt = opt;
	save();
	hide();
	emit options_changed(m_opt);
}

void JAPortalOptionsWindow::apply_gui() {
	JAPortalOptions opt = m_opt;
	
	opt.data_dir = QDir { m_data->le_data_dir->text() };
	if (!opt.data_dir.exists()) {
		QMessageBox::critical(this, "Could Not Apply", "Specified data directory does not exist.");
		return;
	}
	
	opt.output_dir = QDir { m_data->le_output_dir->text() };
	if (!opt.output_dir.exists()) {
		QMessageBox::critical(this, "Could Not Apply", "Specified output directory does not exist.");
		return;
	}
	
	apply(opt);
}
