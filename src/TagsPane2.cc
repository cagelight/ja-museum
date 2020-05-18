#include "TagsPane2.hh"

#include <QMap>

struct TagsPane2::PrivateData {
	QTreeWidgetItem * root;
	QMap<QString, QTreeWidgetItem *> categories;
	QMap<QString, QTreeWidgetItem *> lookup;
};

TagsPane2::TagsPane2(QWidget * parent) : QTreeWidget { parent }, m_data { new PrivateData } {
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMinimumHeight(300);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setColumnCount(2);
	setHeaderLabels({"Tag", "Active"});
	setColumnWidth(0, 310);
	setColumnWidth(1, 0);
	m_data->root = this->invisibleRootItem();
	
	connect(this, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *, int column){
		if (column != 1) return;
		if (!emit_blocker) this->emit_checked_tags();
	});
}

TagsPane2::~TagsPane2() = default;

void TagsPane2::add_tags(QStringList const & tags) {
	emit_blocker = true;
	for (QString const & tag : tags) {
		
		if (m_data->lookup.contains(tag)) continue;
		auto cat = tag.section('_', 0, 0);
		auto name = tag.section('_', 1);
		
		QTreeWidgetItem * parent = nullptr;
		if (m_data->categories.contains(cat))
			parent = m_data->categories[cat];
		else
			parent = m_data->categories[cat] = new QTreeWidgetItem { m_data->root, { cat } };
		
		QTreeWidgetItem * item = new QTreeWidgetItem { parent, { name, nullptr, tag } };
		item->setCheckState(1, Qt::Unchecked);
		m_data->lookup[tag] = item;
	}
	emit_blocker = false;
}

void TagsPane2::remove_tags(QStringList const & tags) {
	emit_blocker = true;
	for (QString const & tag : tags) {
		
		auto iter = m_data->lookup.find(tag);
		if (iter == m_data->lookup.end()) continue;
		auto cat = tag.section('_', 0, 0);
		auto name = tag.section('_', 1);
		
		QTreeWidgetItem * item = *iter;
		m_data->lookup.erase(iter);
		delete item;
		
		auto citer = m_data->categories.find(cat);
		if (citer == m_data->categories.end()) continue;
		if (!citer.value()->childCount()) {
			QTreeWidgetItem * parent = *citer;
			m_data->categories.erase(citer);
			delete parent;
		}
	}
	emit_blocker = false;
}

void TagsPane2::set_tags_checked(QStringList const & tags, bool additive) {
	add_tags(tags);
	emit_blocker = true;
	for (auto iter = m_data->lookup.constBegin(); iter != m_data->lookup.constEnd(); iter++) {
		if (tags.contains(iter.key())) iter.value()->setCheckState(1, Qt::Checked);
		else if (!additive) iter.value()->setCheckState(1, Qt::Unchecked);
	}
	emit_blocker = false;
	emit_checked_tags();
}

void TagsPane2::emit_checked_tags() {
	QStringList checked_tags;
	
	for (auto iter = m_data->lookup.constBegin(); iter != m_data->lookup.constEnd(); iter++) {
		if (iter.value()->checkState(1) != Qt::Checked) continue;
		checked_tags.append(iter.key());
	}
	
	emit tags_checked(checked_tags);
}
