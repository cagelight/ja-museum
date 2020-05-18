const THUMB_EXT = '.webp'
const IMAGE_EXT = '.webp'
const THUMB_DELAY_STATIC = 5000
const THUMB_DELAY_RAND = 20000

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
		} else if (bytes < 2097152) {
			return String((bytes / 1024).toFixed(2)) + ' KiB'
		} else if (bytes < 2147483648) {
			return String((bytes / 1048576).toFixed(2)) + ' MiB'
		} else {
			return String((bytes / 1073741824).toFixed(2)) + ' GiB'
		}
	}
	
	static stringifyTags(itags, etags) {
		let str = ""
		
		if (itags && itags.length && itags[0])
			str += itags.join(';')
		str += ':'
		if (etags && etags.length && etags[0])
			str += etags.join(';')
		return str;
	}
	
	static destringifyTags(str) {
		let stagstr = str.split(':')
		let itags = []
		let etags = []
		
		if (stagstr && stagstr.length && stagstr[0])
			itags = stagstr[0].split(';')
			
		if (stagstr && stagstr.length && stagstr[1])
			etags = stagstr[1].split(';')
		
		return [itags, etags]
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
}

class PageData {
	
	constructor(data) {
		
		this.data = data
		this.sets = this.data.sets
		this.dataLookup = {}
		this.initialTitle = document.title
		
		let calcScore = (set) => {
			let score = 0
			
			if (set.tags.includes('rating_platinum')) score += 400
			if (set.tags.includes('rating_gold'))     score += 200
			if (set.tags.includes('rating_silver'))   score += 100
			if (set.tags.includes('rating_bronze'))   score += 50
			
			/*
			if (set.tags.includes('size_huge')) score += 80
			if (set.tags.includes('size_large')) score += 40
			if (set.tags.includes('size_medium')) score += 20
			if (set.tags.includes('size_small')) score += 10
			if (set.tags.includes('size_tiny')) score += 5
			*/
			
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
			this.dataLookup[set.ident] = set
		}
	}
}

class MainPage {
	
	constructor(pd) {
		this.pageData = pd
		this.sets = this.pageData.sets
		this.data = this.pageData.data
		this.dataElements = []
		this.tags = []
		this.tagElements = []
		this.tagLookup = {}
		
		for (let i = 0; i < this.sets.length; i++) {
			let set = this.sets[i]
			let e = this.createElement(set)
			this.dataElements.push(e)
			main_content.appendChild(e)
			
			for (let ti = 0; ti < set.tags.length; ti++) {
				let tag = set.tags[ti]
				if (!this.tags.includes(tag)) this.tags.push(tag)
			}
		}
		
		this.tags.sort()
		
		let lastcat = ""
		
		for (let i = 0; i < this.tags.length; i++) {
			let tag = this.tags[i]
			
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
	
	configure(itags, etags) {
		
		document.title = this.pageData.initialTitle
		main_content.style.display = null
		filter_panel.style.display = null
		this.filter_entries(itags, etags)
		this.filter_tags(itags, etags)
	}
	
	deconfigure() {
		main_content.style.display = 'none'
		filter_panel.style.display = 'none'
	}
	
	filter_entries(itags, etags) {
		
		if ((!itags || !itags.length || !itags[0]) && (!etags || !etags.length || !etags[0])) {
			for (let i = 0; i < this.dataElements.length; i++) {
				let dat = this.dataElements[i]
				dat.style.display = null
			}
			return;
		}
		
		for (let i = 0; i < this.dataElements.length; i++) {
			let dat = this.dataElements[i]
			dat.style.display = null
			
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
	
	filter_tags(itags, etags) {
		
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
			
			e.addE.href = '#F' + Util.stringifyTags(itags.concat(e.tag), etags)
			e.remE.href = '#F' + Util.stringifyTags(itags, etags.concat(e.tag))
		}
		
	}
	
	createCategory(cat) {
		
		let cont = document.createElement('div')
		cont.className = 'cat_cont'
		
		let namespan = document.createElement('a')
		namespan.className = 'cat_name'
		namespan.innerText = cat
		cont.appendChild(namespan)
		
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
		if (set.tags.includes('rating_gold')) {
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
		thumb_top.style.top = '-120px'
		thumb_bot.className = 'dl_thumb'
		
		let imgs = []
		for (let i = 0; i < set.img.length; i++) {
			imgs[i] = this.data.general.image_root + 't/' + set.ident + '_' + set.img[i] + THUMB_EXT
		}
		
		let imgi = Math.floor(Math.random() * imgs.length)
		let imgi2 = imgi + 1 >= imgs.length ? 0 : imgi + 1
		
		thumb_top.src = imgs[imgi]
		
		if (imgs.length > 1) {
			
			thumb_bot.src = imgs[imgi2]
			imgi = imgi2
			
			let animTimeout = null
			let autoTimeout = null
			let autoTimeoutFunc
			
			let nextImageFunc = () => {
				if (animTimeout != null) {
					return
				}
				autoTimeoutFunc()
				thumb_top.style.animation = "dl_thumb_fade 0.5s forwards"
				animTimeout = setTimeout(()=>{
					animTimeout = null
					thumb_top.style.animation = null
					thumb_top.src = imgs[imgi++]
					if (imgi >= imgs.length) imgi = 0
					
					let imgn = imgi
					if (imgn >= imgs.length) imgn = 0
					thumb_bot.src = imgs[imgn]
				}, 500)
			}
			
			autoTimeoutFunc = () => {
				clearTimeout(autoTimeout)
				autoTimeout = setTimeout(()=>{
					nextImageFunc()
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
	
	constructor(pd) {
		this.pageData = pd
	}
	
	configure(ident) {
		
		document.title = "Details: " + ident
		details_cont.style.display = null
		
		main_content.style.display = 'none'
		details_cont.style.display = null
		gallery.style.display = 'none'
		filter_panel.style.display = 'none'
		
		Util.removeChildren(details_thumbcont)
		let set = this.pageData.dataLookup[ident]
		
		for (let i = 0; i < set.img.length; i++) {
			let imgT = this.pageData.data.general.image_root + 't/' + set.ident + '_' + set.img[i] + THUMB_EXT
			let imgI = this.pageData.data.general.image_root + 'i/' + set.ident + '_' + set.img[i] + IMAGE_EXT
			let img = document.createElement('img')
			img.src = imgT
			img.style.marginLeft = '10px'
			details_thumbcont.appendChild(Util.linkWrap(img, '#I' + ident + '_' + i, 'ilink'))
		}
		
		details_download.href = this.pageData.data.general.content_root + set.path
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
		
		details_titlecont.innerText = set.title ? set.title : "<NO TITLE>"
		details_desccont.innerHTML = set.desc ? set.desc : ""
	}
	
	deconfigure() {
		details_cont.style.display = 'none'
	}
	
}

class GalleryPage {
	
	constructor(pd) {
		this.pageData = pd
	}
	
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
		
		let set = this.pageData.dataLookup[ident]
		
		let imgs = []
		for (let i = 0; i < set.img.length; i++) {
			imgs[i] = this.pageData.data.general.image_root + 'i/' + set.ident + '_' + set.img[i] + IMAGE_EXT
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

var pageMain
var pageDetails
var pageGallery

function configure_page() {
	
	let hash = window.location.hash
	if (hash.length < 2) { // configure main
		pageMain.configure([], [])
		pageDetails.deconfigure()
		pageGallery.deconfigure()
		return
	}
	hash = hash.substr(1)
	if (hash[0] == 'N') { // configure details
		hash = hash.substr(1)
		pageMain.deconfigure()
		pageDetails.configure(hash)
		pageGallery.deconfigure()
	}
	else if (hash[0] == 'I') { // configure gallery
		hash = hash.substr(1)
		pageMain.deconfigure()
		pageDetails.deconfigure()
		pageGallery.configure(hash)
	}
	else if (hash[0] == 'F') { // configure filter
		hash = hash.substr(1)
		let ct = Util.destringifyTags(hash)
		pageMain.configure(ct[0], ct[1])
		pageDetails.deconfigure()
		pageGallery.deconfigure()
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
	let pageData = new PageData(dat)
	
	pageMain = new MainPage(pageData)
	pageDetails = new DetailsPage(pageData)
	pageGallery = new GalleryPage(pageData)
	configure_page()
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
