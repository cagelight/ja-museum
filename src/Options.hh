#pragma once

#include <QDir>
#include <QWidget>

struct JAPortalOptions {
	QDir data_dir;
	QDir output_dir;
};

class JAPortalOptionsWindow : public QWidget {
	Q_OBJECT
public:
	
	JAPortalOptionsWindow(QWidget * parent);
	~JAPortalOptionsWindow();
	
	[[nodiscard]] inline JAPortalOptions const & options() { return m_opt; }
	
signals:
	void options_changed(JAPortalOptions const &);
	
public slots:
	void load();
	void save();
	
private slots:
	void populate_gui();
	
protected:
	void showEvent(QShowEvent *) override;
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
	JAPortalOptions m_opt;
	
	void apply(JAPortalOptions const &);
	void apply_gui();
};
