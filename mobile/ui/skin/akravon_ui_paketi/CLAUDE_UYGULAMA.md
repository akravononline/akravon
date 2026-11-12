# AKRAVON ortak arayüz paketi

Bu paketi oyundaki tüm pencerelere uygulanacak ortak tema olarak kullan. Önce projedeki UI motorunu ve mevcut asset sistemini incele. Görselleri yeniden çizme. İşlevleri ve oyun mantığını değiştirme.

## Dosyalar
- button_normal / selected / pressed / disabled: yazısız dört sekme durumu, 400×128.
- input_normal: stat satırı veya giriş alanı, 400×128.
- portrait_frame: içi şeffaf portre çerçevesi, 256×256.
- plus_normal / pressed / disabled: 200×200.
- close_normal / pressed: 200×200.
- panel_frame: içi şeffaf genel çerçeve, 440×220.
- navy_cloud_texture: lacivert desenli arka plan.
- reference/approved_panel.png: görünüm referansı; bunu çalışan UI yerine tek resim olarak kullanma.

## Ölçekleme ve katmanlar
manifest.json içindeki nine_slice_ltrb sırası sol, üst, sağ, alt; değerler kaynak PNG pikselidir. Çerçevelerin köşelerini sabit tut, orta kenarları uzat. Unity kullanılıyorsa Image Type=Sliced; başka motorda eşdeğer 9-parça yaklaşımını kullan. Desenli butonu aşırı genişletmek deseni uzatır; geniş yüzeylerde ayrı doku + çerçeve kullan.

Katman sırası: koyu lacivert dolgu → desen dokusu → içerik → kenar çerçevesi. Artı ve çarpıyı en üstte ayrı tıklanabilir kontroller olarak tut. Yuvarlak ikonları eşit oranda küçült; en/boy oranını değiştirme. PNG alfalarını koru.

Doku yeniden üretildi; kusursuz dikişsiz tekrar garantisi yok. İlk uygulamada tek doku olarak cover/clip kullan, tekrar döşemeden önce kenar birleşimlerini kontrol et. Yazı arkasında dokuya koyu lacivert bir örtü ekleyerek okunabilirliği artır.

## Etkileşim
Sekme aktif değilken normal, aktifken selected, dokunma sırasında pressed; kullanılamıyorken disabled. Basma bitince önceki aktif durumuna dön. Kapatma butonunda disabled dosyası yok: gerekiyorsa normal varlığın opaklığını düşür. Hover mobil için zorunlu değil.

Tüm yazıları, sayıları, portreyi ve değerleri kodla ekle. Metni PNG içine gömme. Başlık için projenin lisanslı serif fontu, açıklamalar için Türkçe karakter destekleyen okunaklı font kullan. Özgün font dosyası bu pakette yoktur. Renk önerileri manifestte.

Referans ekranın boyutlarını mutlak koordinat olarak kopyalama. Hedef yatay mobil çözünürlükte safe-area ve ölçeklemeyi uygula. Artı/çarpı görseli küçük olsa da dokunma alanını yaklaşık 44–48 mantıksal piksel yap. Buton durumlarının tümünde aynı RectTransform/yerleşim boyutlarını kullan.

## Tüm oyuna yayma
Tek bir Theme/Button/Panel bileşeni oluştur; Karakter, Envanter, Görevler, Ayarlar gibi pencereleri bu ortak bileşenlerden türet. Önce karakter panelinde küçük ekran ve geniş ekran kontrolü yap, sonra diğerlerine uygula. Eski assetleri silmeden bir tema geçişi oluştur.

Bu parçalar referansla uyumlu yeniden üretilmiş görsellerdir; ekranın piksel piksel kesimleri değildir. Tek atlas içinde üretilen durumlar arasında küçük kenar/doku farklılıkları olabilir. 9-slice değerleri başlangıç kesim değerleridir; oyun motorunda gerçek boyutlarda görsel kontrol gerekir. Sunucu bağlantısı veya çalışan oyun kodu pakette bulunmaz.


## V2 tamamlayıcı parçalar
Önceki tüm görseller korunmuştur. Eklenen 12 PNG:
- minus_normal/pressed/disabled: ayarlar ekranındaki azaltma kontrolü.
- arrow_left_normal/pressed ve arrow_right_normal/pressed: sayfalama okları.
- checkbox_off/on: kapalı ve açık seçim kutusu. Basılı durum için kodla hafif renk koyulaştırması uygulanabilir; ayrı basılı PNG yoktur.
- slot_normal/selected: eşya, beceri ve hareket ikonlarının arkasına konulacak 200×200 boş yuvalar. İkonları ayrı katmanda yerleştir.
- list_selected: otomatik av / şebnem gibi seçili satırlar için 400×128 yazısız zemin. Normal durumda input_normal kullan.

Bu PNG'ler önceki malzemeye uyumlu yeni üretimlerdir. Tüm oyunda uygulamadan önce durum geçişlerini gerçek motor içinde kontrol et. Aynı grubun durumlarında sabit boyut ve merkez kullan. Okların ve onay kutularının pasif hâlleri gerekiyorsa kodla renk/opacity uygulanır; ayrı dosya yoktur.

Pop-up eşlemesi: Depo/GM → panel_frame + navy_cloud_texture + button_normal; Yetenekler → input_normal + slot_normal + arrow_*; Hareketler → slot_*; Görevler/Bonuslar → input_normal + button durumları; Ayarlar → checkbox_* + plus_* + minus_*; Oto av → input_normal/list_selected + checkbox_*; Hızlı erişim → slot_* + button durumları.
