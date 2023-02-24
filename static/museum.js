const THUMB_EXT = '.webp'
const IMAGE_EXT = '.webp'
const THUMB_DELAY_STATIC = 5000
const THUMB_DELAY_RAND = 20000

const CAT_DESC = {
	'area': 'This category identifies key areas in maps.',
	'content': 'This category identifies the type of content present in each file.',
	'design': 'This category conveys the overall design choices of maps.',
	'feature': 'This category identifies key features in maps.',
	'rating': 'This category assigns a curator\'s rating to each file, purely personal preference.',
	'size': 'This category assigns a relative "size" for a map. Not just physical size, but also content density are both taken into account. (A giant overly simple map might be considered small in relative "size")',
	'theme': 'This category assigns themes to a map. A map must be fairly strong in a theme to be considered.'
}

const TAG_DESC = {
	'area_arena_duel': 'A type of large duel area with particular arena-like features, at the very least a large viewing area, and sometimes additionally a way to cage-in the arena and/or team entrances.',
	'area_auditorium': 'An area containing a stage, with significant capacity for an audience, without any sort of barrier between audience and stage, and with a backstage area accessible from the stage.',
	'area_dojo': 'An east asian style of room popular in the JA community.',
	'area_duel': 'An area dedicated to dueling, often with a special gimmick.',
	
	'content_adult': 'This file contains adult content.',
	'content_map': 'This file contains one or more compiled maps. (.bsp)',
	'content_map_source': 'This file contains one or more uncompiled maps. (.map)',
	'content_npc': 'This file contains one or more NPC declaration files. (.npc)',
	'content_playermodel': 'This file contains one or more player compatible models.',
	'content_saber': 'This file contains one or more playable sabers. (.sab)',
	'content_species': 'This file contains one or more player compatible models setup to use the JA species system.',
	'content_vehicle': 'This file contains one or more vehicle declaration files. (.veh)',
	
	'design_dense': 'Essentially this tag means the map has a "realistic" and fleshed out layout, not having fake doors, not sprawling the map out into the void, etc. (this tag does not apply to tiny maps or particularly simple small maps)',
	'design_self_contained': 'This map contains a significant interior portion (almost) entirely encapsulated by an exterior portion. (e.g. a fully mapped out ship with exterior and interior.) The interior must be directly reachable from the exterior, and vice versa (no teleporters).',
	
	'feature_destructible': '',
	'feature_easter_egg_hunt': 'There are one or more things hidden around the map to be found, usually just for fun, but sometimes to unlock a secret.',
	'feature_gametype_ctf': 'Team and flag spawns for CTF gameplay.',
	'feature_gametype_duel': 'This file contains a map that is either small enough for decent Duel gameplay, and/or has dedicated Duel spawns. (Note that this tag refers to the Duel (and Powderduel) gametype, not the general concept of duel gameplay.)',
	'feature_gametype_ffa': 'FFA compatible layout and well placed pickups for decent FFA gameplay.',
	'feature_secret': 'Popular in the JA community, secrets are areas of maps that aren\'t found through normal means, usually behind camouflaged doors or invisible teleporters.',
	'feature_secret_many': 'This map has many secrets, this tag usually means secrets were a primary focus of the map.',
	
	'rating_bronze': '',
	'rating_silver': '',
	'rating_gold': '',
	'rating_platinum': '',
	
	'theme_ship': 'A significant portion of the map is on a ship (or space station).',
	'theme_space': 'A significant portion of the map is in space. (NOTE: A space skybox is not enough, part of the map must be open to space.)',
}

class Util {
	
	static linkWrap(elem, url, className = null, target = null) {
		let e = document.createElement('a')
		e.appendChild(elem)
		if (className) e.className = className
		if (target) e.target = target
		e.href = url
		return e
	}
	
	static materialIcon(type) {
		let e = document.createElement('span')
		e.className = 'material-icons'
		e.appendChild(document.createTextNode(type))
		return e
	}
	
	static removeChildren(e) {
		let c = e.lastElementChild
		while (c) {
			e.removeChild(c)
			c = e.lastElementChild
		}
	}
	
	static prettyBytes(bytes) {
		if (bytes < 2048) {
			return String(bytes) + ' B'
		} else if (bytes < 1024000) {
			return String((bytes / 1024).toFixed(2)) + ' KiB'
		} else if (bytes < 1048576000) {
			return String((bytes / 1048576).toFixed(2)) + ' MiB'
		} else {
			return String((bytes / 1073741824).toFixed(2)) + ' GiB'
		}
	}
	
	static stringifyTags(itags, etags, filter) {
		let str = "#F"
		
		if (itags && itags.length && itags[0])
			str += itags.join(',')
		str += ':'
		if (etags && etags.length && etags[0])
			str += etags.join(',')
		str += ':'
		if (filter && filter.length)
			str += encodeURI(filter)
		return str;
	}
	
	static destringifyTags(strin) {
		console.log(strin)
		if (!strin || !strin[0] || strin[0] != '#' || strin[1] != 'F')
			return [[],[],""]
			
		let str = strin.slice(2)
		let stagstr = str.split(':')
		let itags = []
		let etags = []
		let filter = ""
		
		if (stagstr && stagstr.length) {
			if (stagstr[0] && stagstr[0].length)
				itags = stagstr[0].split(',')
			if (stagstr[1] && stagstr[1].length)
				etags = stagstr[1].split(',')
			if (stagstr[2] && stagstr[2].length)
				filter = decodeURI(stagstr[2])
		}
		
		return [itags, etags, filter]
	}
	
	static stringifyHSL(h, s, l) {
		return 'hsl(' + h + ',' + s + '%,' + l + '%)'
	}
	
	static multPcnt(per, m) {
		if (m < 1) return per * m
		return per + ((100 - per) - ((100 - per) * 1 / m))
	}
	
	static setGlobalColors(h, s, l) {
		document.documentElement.style.setProperty('--jap-tint1-f',   Util.stringifyHSL(h, s    , l                   ));
		document.documentElement.style.setProperty('--jap-tint1-l',   Util.stringifyHSL(h, s    , Util.multPcnt(l, 2) ));
		document.documentElement.style.setProperty('--jap-tint1-ud',  Util.stringifyHSL(h, s / 2, l / 2               ));
		document.documentElement.style.setProperty('--jap-tint1-u',   Util.stringifyHSL(h, s / 2, l                   ));
		document.documentElement.style.setProperty('--jap-tint1-ul',  Util.stringifyHSL(h, s / 2, Util.multPcnt(l, 2) ));
	}
	
	static randomizeGlobalColors() {
		Util.setGlobalColors(Math.random() * 360, 100, 50)
	}
	
	static gameColorParse(text) {
		let cont = document.createElement('span')
		cont.className = 'gcol_cont'
		if (text[0] != '^') text = '^7' + text
		let csplit = text.split('^')
		for (let i = 0; i < csplit.length; i++) {
			let color = 7
			let seg = csplit[i]
			if (!seg.length) continue
			if (seg[0] >= '0' && seg[0] <= '9') {
				color = seg[0]
				seg = seg.substr(1)
			}
			let span = document.createElement('span')
			span.className = 'gcol_' + color
			span.appendChild(document.createTextNode(seg))
			cont.appendChild(span)
		}
		
		return cont
	}
	
	static gameColorStrip(text) {
		if (text[0] != '^') text = '^7' + text
		let csplit = text.split('^')
		let ret = ""
		for (let i = 0; i < csplit.length; i++) {
			let seg = csplit[i]
			if (seg[0] >= '0' && seg[0] <= '9') {
				seg = seg.substr(1)
			}
			ret += seg
		}
		return ret
	}
	
	static setupSettingsEntryBoolean(name, element, callback) {
		let ls = localStorage.getItem(name)
		if (ls != null) element.checked = JSON.parse(ls)
		element.addEventListener('change', (e) => {
			callback(element.checked)
			localStorage.setItem(name, JSON.stringify(element.checked))
		})
		callback(element.checked)
	}
	
	static isElementInView(e) {
		let bounds = e.getBoundingClientRect()
		if (bounds.bottom < 0) return false
		if (bounds.top > window.innerHeight) return false
		return true
	}
}

class PageData {
	
	constructor(data) {
		
		this.data = data
		this.sets = this.data.sets
		this.tags = []
		this.dataLookup = {}
		this.initialTitle = document.title
		this.autothumb = true
		this.animthumb = true;
		
		let closure_this = this
		Util.setupSettingsEntryBoolean('settings_autothumb', settings_autothumb_cb, (val) => {
			closure_this.autothumb = val
		})
		Util.setupSettingsEntryBoolean('settings_animthumb', settings_animthumb_cb, (val) => {
			closure_this.animthumb = val
		})
		
		let calcScore = (set) => {
			let score = 0
			
			if (set.tags.includes('rating_platinum')) score += 400
			if (set.tags.includes('rating_gold'))     score += 300
			if (set.tags.includes('rating_silver'))   score += 200
			if (set.tags.includes('rating_bronze'))   score += 100
			
			if (set.tags.includes('content_map')) score += 10 // show maps first for each rating
			
			return score
		}
		
		this.sets.sort((a, b) => {
			let sA = calcScore(a)
			let sB = calcScore(b)
			if (sA == sB) return b.utime - a.utime
			else return sB - sA
		})
		
		for (let i = 0; i < this.sets.length; i++) {
			let set = this.sets[i]
			set.tags.sort()
			this.dataLookup[set.ident] = set
			for (let ti = 0; ti < set.tags.length; ti++) {
				let tag = set.tags[ti]
				if (!this.tags.includes(tag)) this.tags.push(tag)
			}
		}
		
		this.tags.sort()
	}
	
	composeSearchBar(itags, etags, filter) {
		let str = ''
		if (itags.length) for (let i = 0; i < itags.length; i++) {
			str += itags[i] + ' '
		}
		if (etags.length) for (let i = 0; i < etags.length; i++) {
			str += '-' + etags[i] + ' '
		}
		str += filter
		filter_entry.value = str
	}
	
	decomposeSearchBar() {
		let itags = []
		let etags = []
		let filter = ""
		
		let fsplit = filter_entry.value.split(' ')
		for (let i = 0; i < fsplit.length; i++) {
			let field = fsplit[i]
			if (!field.length) continue
			
			let ex = field[0] == '-'
			let tagcheck = ex ? field.substr(1) : field
			if (this.tags.includes(tagcheck)) {
				(ex ? etags : itags).push(tagcheck)
				continue
			}
			
			if (filter.length) filter += ' '
			filter += field
		}
		
		return [itags, etags, filter]
	}
}

class MainPage {
	
	constructor() {
		this.sets = pageData.sets
		this.data = pageData.data
		this.dataElements = []
		this.tagElements = []
		this.tagLookup = {}
		
		for (let i = 0; i < this.sets.length; i++) {
			let set = this.sets[i]
			let e = this.createElement(set)
			this.dataElements.push(e)
			main_content.appendChild(e)
		}
		
		let lastcat = ""
		
		for (let i = 0; i < pageData.tags.length; i++) {
			let tag = pageData.tags[i]
			
			let sp = tag.indexOf('_')
			let cat = tag.substr(0, sp)
			let name = tag.substr(sp + 1)
			
			if (lastcat != cat) {
				lastcat = cat
				tag_cont_main.appendChild(this.createCategory(cat))
			}
			
			let e = this.createTag(tag, name, cat)
			this.tagElements.push(e)
			this.tagLookup[tag] = e
			tag_cont_main.appendChild(e)
		}
	}
	
	configure(itags, etags, text_filter) {
		
		document.title = pageData.initialTitle
		main_content.style.display = null
		filter_panel.style.display = null
		this.filter_entries(itags, etags, text_filter)
		this.filter_tags(itags, etags, text_filter)
	}
	
	deconfigure() {
		main_content.style.display = 'none'
		filter_panel.style.display = 'none'
	}
	
	filter_entries(itags, etags, text_filter) {
		
		for (let i = 0; i < this.dataElements.length; i++) {
			this.dataElements[i].style.display = null
		}
		
		if ((itags && itags.length && itags[0]) || (etags && etags.length && etags[0])) {
			for (let i = 0; i < this.dataElements.length; i++) {
				let dat = this.dataElements[i]
				
				for (let ti = 0; ti < itags.length; ti++) {
					if (!dat.set.tags.includes(itags[ti]))
						dat.style.display = 'none'
				}
				for (let ti = 0; ti < etags.length; ti++) {
					if (dat.set.tags.includes(etags[ti]))
						dat.style.display = 'none'
				}
			}
		}
		
		if (text_filter && text_filter[0]) {
			for (let i = 0; i < this.dataElements.length; i++) {
				let dat = this.dataElements[i]
				
				let match = false
				if (dat.set.path.split('/').slice(-1)[0].toUpperCase().includes(text_filter.toUpperCase()))
					match = true
				if (Util.gameColorStrip(dat.set.title).toUpperCase().includes(text_filter.toUpperCase()))
					match = true
				if (!match) dat.style.display = 'none'
			}
		}
	}
	
	filter_tags(itags, etags, text_filter) {
		
		for (let i = 0; i < this.tagElements.length; i++) {
			let e = this.tagElements[i]
			
			e.style.visibility = null
			if (itags.includes(e.tag) || etags.includes(e.tag)) {
				e.style.visibility = 'hidden'
				continue
			}
			
			e.num.useCount = 0
			
			for (let ei = 0; ei < this.dataElements.length; ei++) {
				let dat = this.dataElements[ei]
				if (dat.style.display !== '') continue
				if (dat.set.tags.includes(e.tag))
					e.num.useCount++
			}
			
			e.num.innerText = e.num.useCount
			
			e.addE.href = Util.stringifyTags(itags.concat(e.tag), etags, text_filter)
			e.remE.href = Util.stringifyTags(itags, etags.concat(e.tag), text_filter)
		}
		
	}
	
	createCategory(cat) {
		
		let cont = document.createElement('div')
		cont.className = 'cat_cont'
		
		let namespan = document.createElement('a')
		namespan.className = 'cat_name'
		namespan.innerText = cat
		cont.appendChild(namespan)
		
		if (cat in CAT_DESC)
			cont.title = CAT_DESC[cat]
		
		return cont
	}
	
	createTag(tag, name, cat) {
		
		let cont = document.createElement('div')
		cont.className = 'tag_cont'
		cont.tag = tag
		
		let namespan = document.createElement('a')
		namespan.className = 'tlink tag_name ' + 'tagspec_' + cat
		namespan.innerText = name
		namespan.href = '#F' + tag
		cont.appendChild(namespan)
		
		let numspan = document.createElement('a')
		numspan.className = 'tag_num'
		numspan.innerText = '0'
		cont.num = numspan
		cont.appendChild(numspan)
		
		let addspan = document.createElement('a')
		addspan.className = 'tlink tag_control'
		addspan.innerText = '\uff0b'
		cont.appendChild(addspan)
		cont.addE = addspan
		
		let remspan = document.createElement('a')
		remspan.className = 'tlink tag_control'
		remspan.innerText = '\uff0d'
		cont.appendChild(remspan)
		cont.remE = remspan
		
		if (tag in TAG_DESC)
			cont.title = TAG_DESC[tag]
		
		return cont
	}
	
	createElement(set) {
		
		let cont = document.createElement('div')
		cont.set = set
		cont.className = 'dl_cont'
		
		let img_stuff = document.createElement('div')
		img_stuff.className = 'dl_img_stuff'
		
		img_stuff.appendChild(
			Util.linkWrap(
				Util.materialIcon('description'),
				'#N' + set.ident,
				'tlink'
			)
		)
		
		img_stuff.appendChild(
			Util.linkWrap(
				Util.materialIcon('save_alt'),
				this.data.general.content_root + set.path,
				'tlink'
			)
		)
		
		img_stuff.appendChild(
			Util.linkWrap(
				Util.materialIcon('collections'),
				'#I' + set.ident,
				'tlink'
			)
		)
		
		let deets = document.createElement('div')
		deets.className = 'dl_deets_cont'
		let deets_size = document.createElement('span')
		deets_size.innerText = Util.prettyBytes(set.size)
		deets_size.className = 'dl_deets_size'
		deets.appendChild(deets_size)
		let deets_star = document.createElement('span')
		
		if (set.tags.includes('rating_platinum')) {
			deets_star.className = 'dl_deets_star_plat'
			deets_star.title = 'Curator\'s Choice: Platinum'
		}
		else if (set.tags.includes('rating_gold')) {
			deets_star.className = 'dl_deets_star_gold'
			deets_star.title = 'Curator\'s Choice: Gold'
		}
		else if (set.tags.includes('rating_silver')) {
			deets_star.className = 'dl_deets_star_silver'
			deets_star.title = 'Curator\'s Choice: Silver'
		}
		else if (set.tags.includes('rating_bronze')) {
			deets_star.className = 'dl_deets_star_bronze'
			deets_star.title = 'Curator\'s Choice: Bronze'
		}
		else 
			deets_star.className = 'dl_deets_star_none'
		
		deets.appendChild(deets_star)
		img_stuff.appendChild(deets)
		
		let thumb_cont = document.createElement('div')
		let thumb_top = document.createElement('img')
		let thumb_bot = document.createElement('img')
		thumb_cont.className = 'dl_thumb_cont'
		thumb_top.className = 'dl_thumb'
		thumb_bot.className = 'dl_thumb'
		
		let imgs = []
		for (let i = 0; i < set.img.length; i++) {
			imgs[i] = this.data.general.image_root + 't/' + set.ident + '_' + set.img[i] + THUMB_EXT
		}
		
		let imgi = Math.floor(Math.random() * imgs.length)
		let imgi2 = imgi + 1 >= imgs.length ? 0 : imgi + 1
		
		thumb_top.src = imgs[imgi]
		
		if (imgs.length > 1) {
			
			thumb_top.style.top = '-120px'
			thumb_bot.src = imgs[imgi2]
			imgi = imgi2
			
			let animTimeout = null
			let autoTimeout = null
			let autoTimeoutFunc
			let pg = pageData
			
			let imageTransitionFunc = () => {
				thumb_top.src = imgs[imgi++]
				if (imgi >= imgs.length) imgi = 0
				
				let imgn = imgi
				if (imgn >= imgs.length) imgn = 0
				thumb_bot.src = imgs[imgn]
			}
			
			let nextImageFunc = () => {
				if (animTimeout != null) {
					return
				}
				autoTimeoutFunc()
				
				if (pg.animthumb) {
					thumb_top.style.animation = "dl_thumb_fade 0.5s forwards"
					animTimeout = setTimeout(()=>{
						animTimeout = null
						thumb_top.style.animation = null
						imageTransitionFunc()
					}, 500)
				} else
					imageTransitionFunc()
			}
			
			autoTimeoutFunc = () => {
				clearTimeout(autoTimeout)
				autoTimeout = setTimeout(()=>{
					if (pg.autothumb && Util.isElementInView(cont)) nextImageFunc()
					autoTimeoutFunc()
				}, THUMB_DELAY_RAND * Math.random() + THUMB_DELAY_STATIC)
			}
			autoTimeoutFunc()
			
			thumb_top.onclick = nextImageFunc
			thumb_top.addEventListener('mousedown', (evt)=>{
				if (evt.which == 2) {
					window.open('#N' + set.ident, '_blank')
				}
			})
			
			thumb_cont.appendChild(thumb_bot)
		}
		
		thumb_cont.appendChild(thumb_top)
		cont.appendChild(thumb_cont)
		cont.appendChild(img_stuff)
		return cont
	}
	
}

class DetailsPage {
	
	constructor() {}
	
	configure(ident) {
		
		document.title = "Details: " + ident
		details_cont.style.display = null
		
		main_content.style.display = 'none'
		details_cont.style.display = null
		gallery.style.display = 'none'
		filter_panel.style.display = 'none'
		
		Util.removeChildren(details_thumbcont)
		let set = pageData.dataLookup[ident]
		
		for (let i = 0; i < set.img.length; i++) {
			let imgT = pageData.data.general.image_root + 't/' + set.ident + '_' + set.img[i] + THUMB_EXT
			let imgI = pageData.data.general.image_root + 'i/' + set.ident + '_' + set.img[i] + IMAGE_EXT
			let img = document.createElement('img')
			img.src = imgT
			img.style.marginLeft = '10px'
			details_thumbcont.appendChild(Util.linkWrap(img, '#I' + ident + '_' + i, 'ilink'))
		}
		
		details_download.href = pageData.data.general.content_root + set.path
		details_gallery.href = '#I' + set.ident
		
		details_name.innerText = set.path.substr(set.path.lastIndexOf('/') + 1)
		//details_ident.innerText = set.ident
		details_size.innerText = Util.prettyBytes(set.size)
		details_added.innerText = new Date(set.utime * 1000).toLocaleDateString()
		details_modified.innerText = new Date(set.mtime * 1000).toLocaleDateString()
		
		details_tags.innerHTML = ""
		
		for (let i = 0; i < set.tags.length; i++) {
			let tag = set.tags[i]
			if (i) details_tags.appendChild(document.createTextNode(' \u2022 '))
			details_tags.appendChild(Util.linkWrap(document.createTextNode(tag), '#F' + tag, 'tlink'))
		}
		
		Util.removeChildren(details_titlecont)
		details_titlecont.appendChild(Util.gameColorParse(set.title ? set.title : "<NO TITLE>"))
		
		details_desccont.innerHTML = set.desc ? set.desc : ""
	}
	
	deconfigure() {
		details_cont.style.display = 'none'
	}
	
}

class GalleryPage {
	
	constructor() {}
	
	configure(ident_in) {
		
		let idspl = ident_in.split('_')
		let ident = idspl[0]
		let start_num = idspl.length >= 2 ? idspl[1] : 0
		
		document.title = "Gallery: " + ident
		gallery.style.display = null
		
		main_content.style.display = 'none'
		details_cont.style.display = 'none'
		gallery.style.display = null
		filter_panel.style.display = 'none'
		
		let set = pageData.dataLookup[ident]
		
		let imgs = []
		for (let i = 0; i < set.img.length; i++) {
			imgs[i] = pageData.data.general.image_root + 'i/' + set.ident + '_' + set.img[i] + IMAGE_EXT
		}
		
		let imgi = parseInt(start_num)
		gallery_image.src = imgs[imgi]
		
		gallery_left.onclick = ()=>{
			imgi--
			if (imgi < 0) imgi = imgs.length - 1
			gallery_image.onload = ()=>{ gallery_image.style.opacity = 1 }
			gallery_image.style.opacity = 0
			gallery_image.src = imgs[imgi]
		}
		
		gallery_right.onclick = ()=>{
			imgi++
			if (imgi >= imgs.length) imgi = 0
			gallery_image.onload = ()=>{ gallery_image.style.opacity = 1 }
			gallery_image.style.opacity = 0
			gallery_image.src = imgs[imgi]
		}
	}
	
	deconfigure() {
		gallery.style.display = 'none'
		gallery_image.src = ""
	}
	
}

function configure_page_filter(ident) {
	main_content.style.display = 'none'
	details_cont.style.display = 'none'
	gallery.style.display = 'none'
}

var pageData
var pageMain
var pageDetails
var pageGallery

function configure_page() {
	
	let hash = window.location.hash
	filter_entry.value = ''
	if (hash.length < 2 || hash[1] == 'F') { // configure main
		let ct = Util.destringifyTags(hash)
		pageMain.configure(ct[0], ct[1], ct[2])
		pageData.composeSearchBar(... ct)
		pageDetails.deconfigure()
		pageGallery.deconfigure()
		return
	}
	if (hash[1] == 'N') { // configure details
		hash = hash.substr(2)
		pageMain.deconfigure()
		pageDetails.configure(hash)
		pageGallery.deconfigure()
	}
	else if (hash[1] == 'I') { // configure gallery
		hash = hash.substr(2)
		pageMain.deconfigure()
		pageDetails.deconfigure()
		pageGallery.configure(hash)
	}
}

function create_material_icon(type) {
	let cont = document.createElement('span')
	cont.className = 'material-icons'
	cont.appendChild(document.createTextNode(type))
	return cont
}

function setup_page(dat) {
	
	Util.randomizeGlobalColors()
	
	pageData = new PageData(dat)
	pageMain = new MainPage(pageData)
	pageDetails = new DetailsPage(pageData)
	pageGallery = new GalleryPage(pageData)
	configure_page()
	
	settings_menu_act.addEventListener('click', (e) => {
		settings_menu.style.visibility = settings_menu.style.visibility == '' ? 'hidden' : ''
	})
	
	filter_entry.addEventListener("keyup", (e)=>{
    if (e.key === "Enter") {
        window.location.href = Util.stringifyTags(... pageData.decomposeSearchBar())
    }
});
}

window.addEventListener('DOMContentLoaded', (e) => {
	fetch('data.json')
	.then(res => res.json())
	.then(data => {
		setup_page(data)
	})
})

window.addEventListener('hashchange', (e) => {
	configure_page()
})

window.addEventListener('keydown', (e) => {
	if (e.key == 'Escape') history.back()
})
