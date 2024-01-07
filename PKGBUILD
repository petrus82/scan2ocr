# This is an example PKGBUILD file. Use this as a start to creating your own,
# and remove these comments. For more information, see 'man PKGBUILD'.
# NOTE: Please fill out the license field for your package! If it is unknown,
# then please put 'unknown'.

# Maintainer: Your Name <youremail@domain.com>
pkgname=scan2ocr
pkgver=0.1
pkgrel=1
epoch=
pkgdesc="transcodes pdf files to black/white, adds an ocr layer and renames them"
arch=()
license=('GPL')
depends=('tesseract' 
	'leptonica'
	'imagemagick'
	'poppler-cpp'
	'libssh'
	'qt6-base'
	'qt6-webengine')

source=("$pkgname-$pkgver.tgz")
md5sums=()
validpgpkeys=()

prepare() {
	cd "$pkgname-$pkgver"
}

build() {
	cd "$pkgname-$pkgver"
	cmake -DCMAKE_BUILD_TYPE=Release .
	make
}

check() {
	cd "$pkgname-$pkgver"
	make -k check
}

package() {
	cd "$pkgname-$pkgver"
	make DESTDIR="$pkgdir/" install
}
