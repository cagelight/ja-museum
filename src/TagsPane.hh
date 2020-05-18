#pragma once

#include <QScrollArea>

class TagsPaneItem : public QWidget {
	Q_OBJECT
	
public:
	TagsPaneItem(QString name, QWidget * parent);
	~TagsPaneItem();
	
	void set_checked(bool);
	bool is_checked() const;
	void set_bg_alt(bool);
	
signals:
	void check_state_changed();
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
	
	QString m_name;
};

class TagsPane : public QScrollArea {
	Q_OBJECT
	
public:
	TagsPane(QWidget * parent);
	~TagsPane();
	
	void add_tags(QStringList const &);
	void remove_tags(QStringList const &);
	void set_tags_checked(QStringList const &, bool additive);
	
signals:
	void tags_checked(QStringList);
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
	
	void rebuild_view();
	void emit_checked_tags();
};
