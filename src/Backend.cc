#include "Backend.hh"
#include "Common.hh"

#include <meadow/aeon.hh>

#include <QDateTime>
#include <QDebug>
#include <QMessageBox>

#include <atomic>
#include <chrono>
#include <iomanip>
#include <map>
#include <memory>
#include <random>
#include <thread>
#include <tuple>
#include <vector>

bool JAPortalDataSet::is_complete() const {
	if (!parent) return false;
	if (path.isEmpty()) return false;
	if (ident.isEmpty()) return false;
	if (images.isEmpty()) return false;
	if (tags.isEmpty()) return false;
	return true;
}

QList<QString> JAPortalDataSet::image_names(char const * ext) const {
	QList<QString> names;
	for (int i : images) {
		names.append(name_for_num(i, ext));
	}
	return names;
}

QString JAPortalDataSet::name_for_num(int num, char const * ext) const {
	return QString { "%1_%2%3" } .arg(ident) .arg(num) .arg(ext);
}

struct JAPortalBackend::PrivateData {
	bool initialized = false;
	JAPortalOptions opt;
	JAPortalGeneralData general;
	std::unordered_map<QString, std::unique_ptr<JAPortalDataSet>> data_sets;
};

JAPortalBackend::JAPortalBackend(QObject * parent) : QObject { parent }, m_data { new PrivateData } {}

JAPortalBackend::~JAPortalBackend() = default;

JAPortalGeneralData & JAPortalBackend::general_data() {
	return m_data->general;
}

bool JAPortalBackend::has_entry(QString const & str) const {
	return m_data->data_sets.find(str) != m_data->data_sets.end();
}

std::string ezhex(void const * data, size_t len) {
	static constexpr uint8_t hmask = 0b11110000;
	static constexpr uint8_t lmask = 0b00001111;
	static constexpr char abase = 87;
	
	std::string str;
	str.resize(len * 2);
	for (size_t i = 0; i < len; i++) {
		char ch = reinterpret_cast<char const *>(data)[i];
		char vh = (ch & hmask) >> 4;
		char vl = ch & lmask;
		str[i*2+0] = static_cast<char>(vh > 9 ? abase + vh : 48 + vh);
		str[i*2+1] = static_cast<char>(vl > 9 ? abase + vl : 48 + vl);
	}
	return str;;
}

static QString generate_identifier() {
	static std::random_device rnd;
	static std::mt19937_64 rng { rnd() };
	
	int64_t v = rng();
	return QString::fromStdString(ezhex(&v, sizeof(v)));
}

JAPortalDataSet const & JAPortalBackend::get_or_create_entry(QString const & str) {
	auto iter = m_data->data_sets.find(str);
	if (iter != m_data->data_sets.end()) return *iter->second;
	
	auto & set = *(m_data->data_sets[str] = std::make_unique<JAPortalDataSet>());
	
	set.parent = &m_data->general;
	set.path = str;
	set.utime = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	while (true) {
		set.ident = generate_identifier();
		
		bool good = true;
		for (auto const & i : m_data->data_sets) {
			if (i.first == set.path) continue;
			if (i.second->ident == set.ident) {
				good = false;
				break;
			}
		}
		if (good) break;
	}
	
	return set;
}

void JAPortalBackend::update_entry_metadata(QMap<QString, QFileInfo> const & fmap) {
	for (auto & entry : m_data->data_sets) {
		auto iter = fmap.find(entry.second->path);
		if (iter == fmap.end()) continue;
		entry.second->mtime = iter->lastModified().toSecsSinceEpoch();
		entry.second->size = iter->size();
	}
}

static bool save_source(QImage src, QString path) {
	
	QFile file { path };
	if (!file.open(QFile::WriteOnly)) return false;
	
	return src.save(&file, SOURCE_SAVE_FORMAT, SOURCE_QUALITY);
}

static bool save_image(QImage src, QString path) {
	
	QFile file { path };
	if (!file.open(QFile::WriteOnly)) return false;
	
	return src.save(&file, IMAGE_SAVE_FORMAT, IMAGE_QUALITY);
}

static bool save_thumbnail(QImage src, QString path) {
	
	QFile file { path };
	if (!file.open(QFile::WriteOnly)) return false;
	
	static constexpr QSize THUMBNAIL_SIZE { THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT };
	QImage thumb = src.scaled(THUMBNAIL_SIZE, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
	if (thumb.size() != THUMBNAIL_SIZE) {
		QPoint point {0, 0};
		if (thumb.width() > THUMBNAIL_WIDTH) { point.setX( (thumb.width() - THUMBNAIL_WIDTH) / 2 ); // center horizontally
		} else point.setY( (thumb.height() - THUMBNAIL_HEIGHT) / 2 ); // center vertically
		thumb = thumb.copy(QRect {point, THUMBNAIL_SIZE});
	}
	
	return thumb.save(&file, THUMB_SAVE_FORMAT, THUMB_QUALITY);
}

bool JAPortalBackend::add_image(JAPortalDataSet const & set, QImage const & img) {
	m_data->opt.output_dir.mkdir(IMAGE_SOURCE_PATH);
	m_data->opt.output_dir.mkdir(IMAGE_FULL_PATH);
	m_data->opt.output_dir.mkdir(IMAGE_THUMB_PATH);
	
	PerfProf perf;
	int num = 0;
	for (;; num++) if (set.images.indexOf(num) == -1) break;
	
	QString spath = QString { "%1/%2/%3" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_SOURCE_PATH) .arg(set.name_for_num(num, SOURCE_SAVE_EXT));
	QString ipath = QString { "%1/%2/%3" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_FULL_PATH) .arg(set.name_for_num(num, IMAGE_SAVE_EXT));
	QString tpath = QString { "%1/%2/%3" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_THUMB_PATH) .arg(set.name_for_num(num, THUMB_SAVE_EXT));
	
	if (!save_source(img, spath)) {
		QMessageBox::critical(nullptr, "Failed To Write Image", QString { "Could not write image to output \"%1\", check permissions." } .arg(spath));
		return false;
	}
	
	if (!save_image(img, ipath)) {
		QMessageBox::critical(nullptr, "Failed To Write Image", QString { "Could not write image to output \"%1\", check permissions." } .arg(ipath));
		return false;
	}
	
	if (!save_thumbnail(img, tpath)) {
		QMessageBox::critical(nullptr, "Failed To Write Image", QString { "Could not write image to output \"%1\", check permissions." } .arg(tpath));
		return false;
	}
	
	const_cast<JAPortalDataSet &>(set).images.append(num);
	perf.mark("Image Add:");
	return true;
}

void JAPortalBackend::remove_image(JAPortalDataSet const & set, int num) {
	if (set.images.indexOf(num) == -1) return;
	const_cast<JAPortalDataSet &>(set).images.removeAll(num);
	
	QString spath = QString { "%1/%2/%3" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_SOURCE_PATH) .arg(set.name_for_num(num, SOURCE_SAVE_EXT));
	QString ipath = QString { "%1/%2/%3" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_FULL_PATH) .arg(set.name_for_num(num, IMAGE_SAVE_EXT));
	QString tpath = QString { "%1/%2/%3" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_THUMB_PATH) .arg(set.name_for_num(num, THUMB_SAVE_EXT));
	
	QFile::remove(spath);
	QFile::remove(ipath);
	QFile::remove(tpath);
}

void JAPortalBackend::purge_and_regenerate_images() {
	
	auto decision = QMessageBox::question(nullptr, "BE SURE DUDE", "Are you sure you want to do this? Are you *really* sure???", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (decision != QMessageBox::Yes) return;
	
	m_data->opt.output_dir.mkdir(IMAGE_FULL_PATH);
	m_data->opt.output_dir.mkdir(IMAGE_THUMB_PATH);
	
	QString sdirp = QString { "%1/%2/" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_SOURCE_PATH);
	QString idirp = QString { "%1/%2/" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_FULL_PATH);
	QString tdirp = QString { "%1/%2/" } .arg(m_data->opt.output_dir.absolutePath()) .arg(IMAGE_THUMB_PATH);
	
	QDir sdir { sdirp };
	QDir idir { idirp };
	QDir tdir { tdirp };
	
	for (auto const & f : idir.entryList(QDir::Files)) idir.remove(f);
	for (auto const & f : tdir.entryList(QDir::Files)) tdir.remove(f);
	
	std::vector<std::tuple<QString, QString, QString>> imgs;
	
	for (auto const & setptr : m_data->data_sets) {
		auto const & set = *setptr.second.get();
		
		auto snames = set.image_names(SOURCE_SAVE_EXT);
		auto inames = set.image_names(IMAGE_SAVE_EXT);
		auto tnames = set.image_names(THUMB_SAVE_EXT);
		
		for (int i = 0; i < snames.size(); i++) {
			imgs.emplace_back(sdirp + snames[i], idirp + inames[i], tdirp + tnames[i]);
		}
	}
	
	std::atomic_uint64_t vcnt {0};
	
	std::vector<std::unique_ptr<std::thread>> workers;
	for (size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
		workers.emplace_back(std::make_unique<std::thread>([&vcnt, &imgs](){
			while (true) {
				uint64_t i = vcnt.fetch_add(1);
				if (i >= imgs.size()) return;
				QImage src { std::get<0>(imgs[i]) };
				save_image(src, std::get<1>(imgs[i]));
				save_thumbnail(src, std::get<2>(imgs[i]));
				
				qDebug() << i << "/" << imgs.size();
			}
		}));
	}
	
	for (auto & thr : workers) thr->join();
}

void JAPortalBackend::set_tags(JAPortalDataSet const & set, QStringList const & tags) {
	const_cast<JAPortalDataSet &>(set).tags = tags;
}

void JAPortalBackend::erase_tags(QStringList const & tags) {
	for (auto & kvp : m_data->data_sets) for (QString const & tag : tags) {
		kvp.second->tags.removeAll(tag);
	}
}

void JAPortalBackend::set_title(JAPortalDataSet const & set, QString const & title) {
	const_cast<JAPortalDataSet &>(set).title = title;
}

void JAPortalBackend::set_description(JAPortalDataSet const & set, QString const & desc) {
	const_cast<JAPortalDataSet &>(set).desc = desc;
}

void JAPortalBackend::load() {
	if (!m_data->initialized) return;
	m_data->general = {};
	m_data->data_sets.clear();
	
	PerfProf perf_import;
	
	QFile in_file { m_data->opt.output_dir.absolutePath() + QDir::separator() + "data.json" };
	if (!in_file.exists()) return;
	if (!in_file.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(nullptr, "Could Not Open", QString { "\"%1\" is not readable" } .arg(m_data->opt.output_dir.absolutePath() + QDir::separator() + "data.json"));
		return;
	}
	
	PerfProf perf_deserialize;
	
	auto bytes = in_file.readAll();
	auto data = meadow::aeon::deserialize_json(bytes.begin(), bytes.end());
	m_data->general.content_root = QString::fromStdString(data["general"]["content_root"].as_string());
	m_data->general.image_root = QString::fromStdString(data["general"]["image_root"].as_string());
	for (auto const & dset : data["sets"].array()) {
		QString path = QString::fromStdString(dset["path"].string());
		QString ident = QString::fromStdString(dset["ident"].string());
		
		auto & set = *(m_data->data_sets[path] = std::make_unique<JAPortalDataSet>());
		set.parent = &m_data->general;
		set.path = path;
		set.ident = ident;
		set.title = QString::fromStdString(dset["title"].string());
		set.desc = QString::fromStdString(dset["desc"].string());
		set.utime = dset["utime"].integer();
		set.mtime = dset["mtime"].integer();
		set.size = dset["size"].integer();
		
		for (auto const & ii : dset["img"].array()) {
			set.images.append(ii.integer());
		}
		
		for (auto const & tag : dset["tags"].array()) {
			set.tags.append(QString::fromStdString(tag.string()));
		}
	}
	
	perf_deserialize.mark("JSON Deserialize:");
	perf_import.mark("Import:");
}

QStringList JAPortalBackend::aggregate_tags() const {
	QSet<QString> tag_set;
	for (auto const & kvp : m_data->data_sets) {
		for (auto const & tag : kvp.second->tags) tag_set.insert(tag);
	};
	QStringList tags;
	for (auto const & tag : tag_set) {
		tags.append(tag);
	}
	std::sort(tags.begin(), tags.end());
	return tags;
}

void JAPortalBackend::save() {
	if (!m_data->initialized) return;
	
	PerfProf perf_export;
	
	QFile out_file { m_data->opt.output_dir.absolutePath() + QDir::separator() + "data.json" };
	if (!out_file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(nullptr, "Could Not Save", QString { "\"%1\" is not writable" } .arg(m_data->opt.output_dir.absolutePath() + QDir::separator() + "data.json"));
		return;
	}
	
	PerfProf perf_serialize;
	
	std::vector<JAPortalDataSet const *> sortset;
	for (auto const & kvp : m_data->data_sets) sortset.emplace_back(kvp.second.get());
	
	struct {
		inline bool operator () (JAPortalDataSet const * a, JAPortalDataSet const * b) {
			return a->utime > b->utime;
		}
	} set_comp;
	
	std::sort(sortset.begin(), sortset.end(), set_comp);
	
	meadow::aeon out;
	auto & general = out["general"];
	general["content_root"] = m_data->general.content_root.toStdString();
	general["image_root"] = m_data->general.image_root.toStdString();
	auto & sets = out["sets"].array();
	for (JAPortalDataSet const * data : sortset) {
		auto & set = sets.emplace_back().map();
		set["path"] = data->path.toStdString();
		set["ident"] = data->ident.toStdString();
		set["title"] = data->title.toStdString();
		set["desc"] = data->desc.toStdString();
		set["utime"] = data->utime;
		set["mtime"] = data->mtime;
		set["size"] = data->size;
		
		auto & imgs = set["img"].array();
		for (int i : data->images) imgs.emplace_back(i);
		
		auto & tags = set["tags"].array();
		for (auto const & tag : data->tags) tags.emplace_back(tag.toStdString());
	}
	
	std::string json = out.serialize_json();
	perf_serialize.mark("JSON Serialize:");
	out_file.write(json.data(), json.size());
	perf_export.mark("Export:");
}

void JAPortalBackend::prepare(JAPortalOptions const & opt) {
	m_data->initialized = true;
	m_data->opt = opt;
	load();
}

void JAPortalBackend::validate() {
	
	struct Validator {
		void add_implication(QString const & a, QString const & b) { implications[a].insert(b); }
		void add_bidirectional_implication(QString const & a, QString const & b) {
			add_implication(a, b);
			add_implication(b, a);
		}
		void resolve(QSet<QString> & tags) {
			while (true) {
				bool imade = false;
				for (auto iter = implications.begin(); iter != implications.end(); iter++) {
					if (tags.contains(iter.key())) {
						for (auto & tag : iter.value()) {
							if (tags.contains(tag)) continue;
							tags.insert(tag);
							imade = true;
						}
					}
				}
				if (!imade) break;
			}
		}
	private:
		QMap<QString, QSet<QString>> implications;
	};
	
	for (auto & setptr : m_data->data_sets) {
		JAPortalDataSet & set = *setptr.second;
		QSet<QString> tags { set.tags.begin(), set.tags.end() };
		
		Validator v;
		v.add_implication("area_auditorium", "area_stage");
		v.add_implication("area_dojo", "area_duel");
		v.add_implication("area_arena_duel", "area_duel");
		v.add_implication("area_arena_flying", "area_flying");
		v.add_implication("area_race_track", "area_driving");
		v.add_implication("area_sport_jousting", "area_driving");
		v.add_implication("area_flying", "feature_vehicle");
		v.add_implication("area_driving", "feature_vehicle");
		v.add_implication("area_rancor_pit", "feature_npc_combat");
		v.add_implication("area_npc_fight", "feature_npc_combat");
		v.add_implication("area_cave_crystal", "area_cave");
		v.add_implication("content_species", "content_playermodel");
		v.add_implication("feature_duel_spawns", "feature_gametype_duel");
		v.add_implication("feature_secret_many", "feature_secret");
		v.resolve(tags);
		
		set.tags = QStringList { tags.begin(), tags.end() };
	}
}
