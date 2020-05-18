#pragma once

#include "Common.hh"

#include <QTreeWidget>

class TagsPane2 : public QTreeWidget {
	Q_OBJECT
	
public:
	TagsPane2(QWidget * parent);
	~TagsPane2();
	
	void add_tags(QStringList const &);
	void remove_tags(QStringList const &);
	void set_tags_checked(QStringList const &, bool additive);
	
signals:
	void tags_checked(QStringList);
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
	bool emit_blocker = false;
	
private slots:
	void emit_checked_tags();
};
