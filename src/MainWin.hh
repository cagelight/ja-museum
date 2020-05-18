#pragma once

#include "Common.hh"
#include "Options.hh"
#include "Backend.hh"

#include <QMainWindow>

#include <memory>

class QTreeWidgetItem;

class JAPortalMainWin : public QMainWindow {
	Q_OBJECT
public:
	
	JAPortalMainWin();
	~JAPortalMainWin();
	
private slots:
	void populate_gui(JAPortalOptions const & opt);
	void setup_current_item();
	void add_image_to_current_item(QString);
	void set_tags_on_current_item(QStringList);
	void set_title_on_current_item(QString);
	void set_description_on_current_item(QString);
	void save();
	void select_random_item();
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
	JAPortalOptionsWindow * m_opt_win;
	JAPortalBackend * m_backend;
	
	void update_item_status(QTreeWidgetItem * item);
};
