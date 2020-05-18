#include "TagsPane.hh"

#include <QMap>
#include <QSet>

#include <QCheckBox>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>

struct TagsPaneItem::PrivateData {
	QCheckBox * check = nullptr;
};

TagsPaneItem::TagsPaneItem(QString name, QWidget * parent) : QWidget { parent }, m_data { new PrivateData }, m_name { name } {
	QHBoxLayout * lay = new QHBoxLayout { this };
	lay->setMargin(0);
	lay->setSpacing(0);
	
	QLabel * tag_label = new QLabel { name, this };
	tag_label->setStyleSheet("padding: 2px 0px;");
	tag_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	lay->addWidget(tag_label);
	
	QWidget * spacer = new QWidget { this };
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	lay->addWidget(spacer);
	
	m_data->check = new QCheckBox { this };
	m_data->check->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	lay->addWidget(m_data->check);
	
	connect(m_data->check, &QCheckBox::stateChanged, this, &TagsPaneItem::check_state_changed);
	
	set_bg_alt(false);
}

TagsPaneItem::~TagsPaneItem() = default;

void TagsPaneItem::set_checked(bool ch) {
	m_data->check->setChecked(ch);
}

bool TagsPaneItem::is_checked() const {
	return m_data->check->isChecked();
}

void TagsPaneItem::set_bg_alt(bool alt) {
	this->setStyleSheet(alt ? "background-color:palette(alternate-base)" : "background-color:palette(base)");
}


struct TagsPane::PrivateData {
	QMap<QString, TagsPaneItem *> tag_items;
	
	QWidget * wid = nullptr;
	QVBoxLayout * lay = nullptr;
};

TagsPane::TagsPane(QWidget * parent) : QScrollArea { parent }, m_data { new PrivateData } {
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMinimumHeight(300);
	setWidgetResizable(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	
	m_data->wid = new QWidget { this };
	setWidget(m_data->wid);
}

TagsPane::~TagsPane() = default;

void TagsPane::add_tags(QStringList const & tags) {
	bool tags_added = false;
	for (QString const & tag : tags) {
		if (m_data->tag_items.find(tag) != m_data->tag_items.end()) continue;
		TagsPaneItem * item = m_data->tag_items[tag] = new TagsPaneItem { tag, this };
		connect(item, &TagsPaneItem::check_state_changed, this, &TagsPane::emit_checked_tags);
		tags_added = true;
	}
	if (tags_added) rebuild_view();
}

void TagsPane::remove_tags(QStringList const & tags) {
	bool tags_removed = false;
	bool checked_tags_removed = false;
	for (QString const & tag : tags) {
		auto iter = m_data->tag_items.find(tag);
		if (iter == m_data->tag_items.end()) continue;
		tags_removed = true;
		if (iter.value()->is_checked()) checked_tags_removed = true;
		delete iter.value();
		m_data->tag_items.erase(iter);
	}
	if (tags_removed) rebuild_view();
	if (checked_tags_removed) emit_checked_tags();
}

void TagsPane::set_tags_checked(QStringList const & tags, bool additive) {
	
	QSet<QString> tagset {tags.begin(), tags.end()};
	for (auto iter = m_data->tag_items.begin(); iter != m_data->tag_items.end(); iter++) {
		if (tagset.find(iter.key()) != tagset.end()) iter.value()->set_checked(true);
		else if (!additive) iter.value()->set_checked(false);
	}
}

void TagsPane::rebuild_view() {
	if (m_data->lay) delete m_data->lay;
	m_data->lay = new QVBoxLayout { m_data->wid };
	m_data->lay->setAlignment(Qt::AlignTop);
	m_data->lay->setSpacing(0);
	m_data->lay->setMargin(0);
	
	bool alt = false;
	for (auto iter = m_data->tag_items.begin(); iter != m_data->tag_items.end(); iter++) {
		iter.value()->set_bg_alt(alt);
		m_data->lay->addWidget(iter.value());
		alt = !alt;
	}
}

void TagsPane::emit_checked_tags() {
	QStringList checked_tags;
	for (auto iter = m_data->tag_items.begin(); iter != m_data->tag_items.end(); iter++) {
		if (iter.value()->is_checked()) checked_tags << iter.key();
	}
	emit tags_checked(checked_tags);
}
