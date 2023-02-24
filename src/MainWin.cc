#include "MainWin.hh"
#include "ImagePane.hh"
#include "TagsPane2.hh"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <random>

struct JAPortalMainWin::PrivateData {
	QTreeWidget * pk3_view;
	QMap<QString, QFileInfo> pk3_view_infos;
	QMap<QString, QTreeWidgetItem *> pk3_view_item_lookup;
	
	bool pk3_view_unproc_hidden = false;
	bool pk3_view_proc_hidden = false;
	QString pk3_view_name_filter;
	
	QWidget * ci_info_cont = nullptr;
	QLineEdit * ci_title = nullptr;
	QTextEdit * ci_desc = nullptr;
	
	QLineEdit * content_prefix_le = nullptr;
	QLineEdit * image_prefix_le = nullptr;
	
	QLineEdit * ci_pre_name_le = nullptr;
	
	QTreeWidgetItem * ci_item_sel = nullptr;
	QTreeWidgetItem * ci_item = nullptr;
	QWidget * ci_wid = nullptr;
	ImagePane * ci_img_pane = nullptr;
	TagsPane2 * ci_tags = nullptr;
};

JAPortalMainWin::JAPortalMainWin() : QMainWindow(), m_data { new PrivateData } {
	
	m_opt_win = new JAPortalOptionsWindow { this };
	m_backend = new JAPortalBackend { this };
	
	// ================
	
	auto menu_special = this->menuBar()->addMenu("Special");
	auto menu_special_random = menu_special->addAction("Select Random Item");
	connect(menu_special_random, &QAction::triggered, this, &JAPortalMainWin::select_random_item);
	auto menu_special_regen = menu_special->addAction("Regenerate Images (EXPENSIVE)");
	connect(menu_special_regen, &QAction::triggered, m_backend, &JAPortalBackend::purge_and_regenerate_images);
	
	auto menu_settings = this->menuBar()->addMenu("Settings");
	auto menu_settings_conf = menu_settings->addAction("Configure JAPortal");
	connect(menu_settings_conf, &QAction::triggered, m_opt_win, &QWidget::show);
	
	// ================
	
	QWidget * main_wid = new QWidget { this };
	QGridLayout * main_lay = new QGridLayout { main_wid };
	m_data->pk3_view = new QTreeWidget { main_wid };
	m_data->pk3_view->setMinimumSize(600, 300);
	m_data->pk3_view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	main_lay->addWidget(m_data->pk3_view, 1, 0);
	
	QWidget * pk3_ctrl_wid = new QWidget { main_wid };
	QHBoxLayout * pk3_ctrl_lay = new QHBoxLayout { pk3_ctrl_wid };
	pk3_ctrl_lay->setContentsMargins(0, 0, 0, 0);
	QLineEdit * pk3_ctrl_filter = new QLineEdit { pk3_ctrl_wid };
	QCheckBox * pk3_ctrl_hide_proc = new QCheckBox { pk3_ctrl_wid };
	QCheckBox * pk3_ctrl_hide_unproc = new QCheckBox { pk3_ctrl_wid };
	pk3_ctrl_lay->addWidget( new QLabel { "Filter:", pk3_ctrl_wid } );
	pk3_ctrl_lay->addWidget(pk3_ctrl_filter);
	pk3_ctrl_lay->addWidget( new QLabel { "Hide Complete:", pk3_ctrl_wid } );
	pk3_ctrl_lay->addWidget(pk3_ctrl_hide_proc);
	pk3_ctrl_lay->addWidget( new QLabel { "Hide Unprocessed:", pk3_ctrl_wid } );
	pk3_ctrl_lay->addWidget(pk3_ctrl_hide_unproc);
	main_lay->addWidget(pk3_ctrl_wid, 0, 0);
	
	connect(pk3_ctrl_filter, &QLineEdit::textChanged, this, [this](QString const & text){
		m_data->pk3_view_name_filter = text;
		filter_pk3_view();
	});
	
	connect(pk3_ctrl_hide_proc, &QCheckBox::stateChanged, this, [this](int state){
		m_data->pk3_view_proc_hidden = state & Qt::Checked;
		filter_pk3_view();
	});
	
	connect(pk3_ctrl_hide_unproc, &QCheckBox::stateChanged, this, [this](int state){
		m_data->pk3_view_unproc_hidden = state & Qt::Checked;
		filter_pk3_view();
	});
	
	QWidget * right_area = new QWidget { main_wid };
	right_area->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	QVBoxLayout * right_lay = new QVBoxLayout { right_area };
	right_lay->setContentsMargins(0, 0, 0, 0);
	right_lay->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	main_lay->addWidget(right_area, 0, 1, 3, 1);
	
	m_data->ci_info_cont = new QWidget { main_wid };
	QGridLayout * info_lay = new QGridLayout { m_data->ci_info_cont };
	info_lay->setContentsMargins(0, 0, 0, 0);
	m_data->ci_title = new QLineEdit { m_data->ci_info_cont };
	m_data->ci_title->setPlaceholderText("Title");
	info_lay->addWidget(m_data->ci_title, 0, 0);
	m_data->ci_desc = new QTextEdit { m_data->ci_info_cont };
	m_data->ci_desc->setAcceptRichText(false);
	m_data->ci_desc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_data->ci_desc->setPlaceholderText("Description");
	info_lay->addWidget(m_data->ci_desc, 1, 0);
	main_lay->addWidget(m_data->ci_info_cont, 2, 0);
	
	connect(m_data->pk3_view, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem * item, int){
		if (!item) return;
		if (item->text(1).isEmpty()) return;
		m_data->ci_item = item;
		setup_current_item();
	});
	
	connect(m_data->ci_title, &QLineEdit::editingFinished, this, [this](){
		this->set_title_on_current_item(m_data->ci_title->text());
	});
	
	connect(m_data->ci_desc, &QTextEdit::textChanged, this, [this](){
		this->set_description_on_current_item(m_data->ci_desc->toPlainText());
	});
	
	// ================
	
	QWidget * content_prefix_cont = new QWidget { right_area };
	QHBoxLayout * content_prefix_lay = new QHBoxLayout { content_prefix_cont };
	content_prefix_lay->setContentsMargins(0, 0, 0, 0);
	content_prefix_lay->addWidget(new QLabel { "Content Root:", content_prefix_cont });
	m_data->content_prefix_le = new QLineEdit { content_prefix_cont };
	m_data->content_prefix_le->setMinimumWidth(300);
	content_prefix_lay->addWidget(m_data->content_prefix_le);
	right_lay->addWidget(content_prefix_cont);
	
	connect(m_data->content_prefix_le, &QLineEdit::editingFinished, this, [this](){
		m_backend->general_data().content_root = m_data->content_prefix_le->text();
	});
	
	// ================
	
	QWidget * image_prefix_cont = new QWidget { right_area };
	QHBoxLayout * image_prefix_lay = new QHBoxLayout { image_prefix_cont };
	image_prefix_lay->setContentsMargins(0, 0, 0, 0);
	image_prefix_lay->addWidget(new QLabel { "Image Root:", image_prefix_cont });
	m_data->image_prefix_le = new QLineEdit { image_prefix_cont };
	m_data->image_prefix_le->setMinimumWidth(300);
	image_prefix_lay->addWidget(m_data->image_prefix_le);
	right_lay->addWidget(image_prefix_cont);
	
	connect(m_data->image_prefix_le, &QLineEdit::editingFinished, this, [this](){
		m_backend->general_data().image_root = m_data->image_prefix_le->text();
	});
	
	// ================
	
	QWidget * ci_pre_cont = new QWidget { right_area };
	QHBoxLayout * ci_pre_lay = new QHBoxLayout { ci_pre_cont };
	ci_pre_lay->setContentsMargins(0, 0, 0, 0);
	m_data->ci_pre_name_le = new QLineEdit { ci_pre_cont };
	m_data->ci_pre_name_le->setReadOnly(true);
	m_data->ci_pre_name_le->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	QPushButton * ci_begin_b = new QPushButton { "Begin", ci_pre_cont };
	ci_begin_b->setEnabled(false);
	ci_pre_lay->addWidget(ci_begin_b);
	ci_pre_lay->addWidget(m_data->ci_pre_name_le);
	right_lay->addWidget(ci_pre_cont);
	
	connect(ci_begin_b, &QPushButton::clicked, this, [this](){
		if (!m_data->ci_item_sel) return;
		m_data->ci_item = m_data->ci_item_sel;
		setup_current_item();
	});
	
	// ================
	
	m_data->ci_wid = new QWidget { right_area };
	m_data->ci_wid->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	right_lay->addWidget(m_data->ci_wid);
	
	QVBoxLayout * ci_lay = new QVBoxLayout { m_data->ci_wid };
	ci_lay->setContentsMargins(0, 0, 0, 0);
	ci_lay->setAlignment(Qt::AlignTop);
	
	m_data->ci_img_pane = new ImagePane { m_data->ci_wid };
	ci_lay->addWidget(m_data->ci_img_pane);
	
	QPushButton * ci_add_img_b = new QPushButton { "Add Image", m_data->ci_wid };
	ci_lay->addWidget(ci_add_img_b);
	
	m_data->ci_tags = new TagsPane2 { m_data->ci_wid };
	ci_lay->addWidget(m_data->ci_tags);
	
	QWidget * ci_tagsb_wid = new QWidget { m_data->ci_wid };
	QHBoxLayout * ci_tagsb_lay = new QHBoxLayout { ci_tagsb_wid };
	ci_tagsb_lay->setContentsMargins(0, 0, 0, 0);
	QPushButton * ci_tagsb_add_b = new QPushButton { "Add Tags", ci_tagsb_wid };
	QPushButton * ci_tagsb_rem_b = new QPushButton { "Remove Tags", ci_tagsb_wid };
	QPushButton * ci_tagsb_set_b = new QPushButton { "Set Tags", ci_tagsb_wid };
	ci_tagsb_lay->addWidget(ci_tagsb_rem_b);
	ci_tagsb_lay->addWidget(ci_tagsb_add_b);
	ci_tagsb_lay->addWidget(ci_tagsb_set_b);
	ci_lay->addWidget(ci_tagsb_wid);
	
	connect(m_data->ci_img_pane, &ImagePane::image_dropped, this, &JAPortalMainWin::add_image_to_current_item);
	connect(m_data->ci_img_pane, &ImagePane::delete_request, this, [this](JAPortalDataSet const * dset, int num){
		Q_ASSERT(m_data->ci_item);
		JAPortalDataSet const & data = m_backend->get_or_create_entry(m_data->ci_item->text(1));
		Q_ASSERT(dset == &data);
		this->delete_image_on_current_item(num);
	});
	
	connect(ci_add_img_b, &QPushButton::clicked, this, [this](){
		static QString last_dir = QDir::homePath();
		QStringList imgs = QFileDialog::getOpenFileNames(m_data->ci_wid, "Select Images", last_dir, "Supported Lossless Images (*.bmp *.png *.tga *.webp))");
		if (imgs.isEmpty()) return;
		last_dir = QFileInfo { imgs[0] } .absolutePath();
		for (QString path : imgs) {
			add_image_to_current_item(path);
		}
	});
	
	connect(ci_tagsb_add_b, &QPushButton::clicked, this, [this](){
		QString tagstr = QInputDialog::getText(this, "Add Tags", "Enter tags to be added, separated by space:");
		if (tagstr.isEmpty()) return;
		QStringList tags = tagstr.split(" ", Qt::SkipEmptyParts);
		if (tags.isEmpty()) return;
		
		m_data->ci_tags->add_tags(tags);
	});
	
	connect(ci_tagsb_rem_b, &QPushButton::clicked, this, [this](){
		QString tagstr = QInputDialog::getText(this, "Remove Tags", "Enter tags to be removed, separated by space:");
		if (tagstr.isEmpty()) return;
		QStringList tags = tagstr.split(" ", Qt::SkipEmptyParts);
		if (tags.isEmpty()) return;
		
		m_data->ci_tags->remove_tags(tags);
		m_backend->erase_tags(tags);
	});
	
	connect(ci_tagsb_set_b, &QPushButton::clicked, this, [this](){
		QString tagstr = QInputDialog::getText(this, "Set Tags", "Enter tags to be checked (and added if not present), separated by space:");
		if (tagstr.isEmpty()) return;
		QStringList tags = tagstr.split(" ", Qt::SkipEmptyParts);
		if (tags.isEmpty()) return;
		
		m_data->ci_tags->add_tags(tags);
		m_data->ci_tags->set_tags_checked(tags, true);
	});
	
	connect(m_data->ci_tags, &TagsPane2::tags_checked, this, &JAPortalMainWin::set_tags_on_current_item);
	
	// ================
	
	QWidget * button_cont = new QWidget { right_area };
	button_cont->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	QHBoxLayout * button_lay = new QHBoxLayout { button_cont };
	button_lay->setContentsMargins(0, 0, 0, 0);
	QPushButton * validate_b = new QPushButton { "Validate", button_cont };
	QPushButton * save_b = new QPushButton { "Save && Export", button_cont };
	button_lay->addWidget(validate_b);
	button_lay->addWidget(save_b);
	right_lay->addWidget(button_cont);
	
	connect(validate_b, &QPushButton::clicked, this, &JAPortalMainWin::validate);
	connect(save_b, &QPushButton::clicked, this, &JAPortalMainWin::save);
	
	// ================
	
	
	connect(m_data->pk3_view, &QTreeWidget::currentItemChanged, this, [this, ci_begin_b](QTreeWidgetItem * cur, QTreeWidgetItem *){
		if (!cur || cur->text(1).isEmpty()) {
			m_data->ci_item_sel = nullptr;
			ci_begin_b->setEnabled(false);
			return;
		}
		m_data->ci_item_sel = cur;
		ci_begin_b->setEnabled(true);
	});
	
	this->setCentralWidget(main_wid);
	
	connect(m_opt_win, &JAPortalOptionsWindow::options_changed, m_data->ci_img_pane, &ImagePane::prepare);
	connect(m_opt_win, &JAPortalOptionsWindow::options_changed, m_backend, &JAPortalBackend::prepare);
	connect(m_opt_win, &JAPortalOptionsWindow::options_changed, this, &JAPortalMainWin::populate_gui);
	
	m_opt_win->load();
}

JAPortalMainWin::~JAPortalMainWin() {
	
}

void JAPortalMainWin::update_item_status(QTreeWidgetItem * item) {
	if (m_backend->has_entry(item->text(1))) {
		auto const & entry = m_backend->get_or_create_entry(item->text(1));
		if (entry.is_complete()) {
			item->setText(2, "Complete");
			item->setForeground(2, QColor::fromRgb(64, 255, 64));
		} else {
			item->setText(2, "Incomplete");
			item->setForeground(2, QColor::fromRgb(255, 255, 64));
		}
	} else {
		item->setText(2, "Unprocessed");
		item->setForeground(2, QColor::fromRgb(255, 64, 64));
	}
}

void JAPortalMainWin::setup_current_item() {
	if (!m_data->ci_item) {
		m_data->ci_wid->setEnabled(false);
		m_data->ci_info_cont->setEnabled(false);
		return;
	}
	
	m_data->ci_wid->setEnabled(true);
	m_data->ci_info_cont->setEnabled(true);
	
	m_data->ci_pre_name_le->setText(m_data->ci_item->text(1));
	
	JAPortalDataSet const & data = m_backend->get_or_create_entry(m_data->ci_item->text(1));
	
	m_data->ci_title->setText(data.title);
	m_data->ci_desc->setText(data.desc);
	
	m_data->ci_img_pane->clear();
	
	for (int i = 0; i < data.images.size(); i++) {
		m_data->ci_img_pane->add_image(&data, i);
	}
	m_data->ci_tags->set_tags_checked(data.tags, false);
	update_item_status(m_data->ci_item);
}

void JAPortalMainWin::add_image_to_current_item(QString path) {
	QImage img { path };
	if (img.isNull()) {
		QMessageBox::critical(this, "Could Not Load Image", QString { "Failed to read image: %1" } .arg(path));
		return;
	}
	JAPortalDataSet const & data = m_backend->get_or_create_entry(m_data->ci_item->text(1));
	if (!m_backend->add_image(data, img)) {
		return;
	}
	m_data->ci_img_pane->add_image(&data, data.images.last());
	update_item_status(m_data->ci_item);
}

void JAPortalMainWin::delete_image_on_current_item(int num) {
	if (!m_data->ci_item) return;
	m_backend->remove_image(m_backend->get_or_create_entry(m_data->ci_item->text(1)), num);
}

void JAPortalMainWin::set_tags_on_current_item(QStringList tags) {
	JAPortalDataSet const & data = m_backend->get_or_create_entry(m_data->ci_item->text(1));
	m_backend->set_tags(data, tags);
	update_item_status(m_data->ci_item);
}

void JAPortalMainWin::set_title_on_current_item(QString title) {
	JAPortalDataSet const & data = m_backend->get_or_create_entry(m_data->ci_item->text(1));
	m_backend->set_title(data, title);
	update_item_status(m_data->ci_item);
}

void JAPortalMainWin::set_description_on_current_item(QString desc) {
	JAPortalDataSet const & data = m_backend->get_or_create_entry(m_data->ci_item->text(1));
	m_backend->set_description(data, desc);
	update_item_status(m_data->ci_item);
}

void JAPortalMainWin::validate() {
	m_backend->validate();
	setup_current_item();
}

void JAPortalMainWin::save() {
	validate();
	m_backend->update_entry_metadata(m_data->pk3_view_infos);
	m_backend->save();
}

void JAPortalMainWin::select_random_item() {
	std::vector<QTreeWidgetItem *> candidates;
	for (auto const & kvp : m_data->pk3_view_item_lookup)
		if (kvp->text(2) == "Incomplete") candidates.emplace_back(kvp);
	if (!candidates.size()) for (auto const & kvp : m_data->pk3_view_item_lookup)
		if (kvp->text(2) == "Unprocessed") candidates.emplace_back(kvp);
	if (!candidates.size()) {
		QMessageBox::information(this, "Nothing Left", "Nothing Left To Do");
		return;
	}
	
	static std::mt19937_64 rng {std::random_device{}()};
	std::uniform_int_distribution<size_t> dist { 0, candidates.size() - 1 };
	
	QTreeWidgetItem * item = candidates[dist(rng)];
	m_data->pk3_view->expandAll();
	m_data->pk3_view->setCurrentItem(item);
}

void JAPortalMainWin::filter_pk3_view() {
	for (auto & item : m_data->pk3_view_item_lookup) {
		bool hide = false;
		if (m_data->pk3_view_proc_hidden && item->text(2) == "Complete") hide = true;
		if (m_data->pk3_view_unproc_hidden && item->text(2) == "Unprocessed") hide = true;
		if (!item->text(1).contains(m_data->pk3_view_name_filter, Qt::CaseInsensitive)) hide = true;
		item->setHidden(hide);
	}
}

void JAPortalMainWin::populate_gui(JAPortalOptions const & opt) {
	PerfProf perf;
	m_data->ci_item = nullptr;
	m_data->pk3_view->clear();
	m_data->pk3_view_item_lookup.clear();
	m_data->pk3_view_infos.clear();
	QTreeWidgetItem * root = m_data->pk3_view->invisibleRootItem();
	std::function<void(QTreeWidgetItem *, QDir)> recursor = [this, &recursor, &opt](QTreeWidgetItem * parent, QDir working_dir) {
		for (auto const & entry : working_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			QTreeWidgetItem * item = new QTreeWidgetItem;
			parent->addChild(item);
			item->setText(0, entry.fileName());
			recursor(item, entry.absoluteFilePath());
		}
		for (auto const & entry : working_dir.entryInfoList(QDir::Files)) {
			QTreeWidgetItem * item = new QTreeWidgetItem;
			parent->addChild(item);
			auto rel_path = opt.data_dir.relativeFilePath(entry.absoluteFilePath());
			m_data->pk3_view_item_lookup[rel_path] = item;
			m_data->pk3_view_infos[rel_path] = entry;
			item->setText(0, entry.fileName());
			item->setText(1, rel_path);
			update_item_status(item);
		}
	};
	recursor(root, opt.data_dir);
	m_data->pk3_view->setColumnCount(3);
	m_data->pk3_view->setHeaderLabels({"File", "Relative Path", "Processed?"});
	m_data->pk3_view->setColumnWidth(0, 300);
	m_data->pk3_view->setColumnWidth(1, 160);
	m_data->pk3_view->expandAll();
	filter_pk3_view();
	
	auto const & globals = m_backend->general_data();
	m_data->content_prefix_le->setText(globals.content_root);
	m_data->image_prefix_le->setText(globals.image_root);
	
	m_data->ci_tags->add_tags(m_backend->aggregate_tags());
	
	setup_current_item();
	perf.mark("GUI Setup:");
}
