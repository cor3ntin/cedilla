#include <ucd/decomposition.h>
#include "unilib/unilib/uninorms.h"
#include <ogonek/normalization.h++>

extern const std::u32string data;

#if 0
int main() {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    for(int i = 0; i < 1000; i++) {
        unicode::normalized(str, unicode::NormalizationForm::NFC);
    }
}

#else

#    define NONIUS_RUNNER
#    include <nonius/nonius_single.h++>


NONIUS_BENCHMARK("ucd_nfd-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    meter.measure([&]() { unicode::normalized(data, unicode::NormalizationForm::NFC); });
})

NONIUS_BENCHMARK("ucd_nfd-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    meter.measure([&]() { unicode::normalized(data, unicode::NormalizationForm::NFD); });
})

NONIUS_BENCHMARK("ucd_nfc-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    meter.measure([&]() { unicode::normalized(data, unicode::NormalizationForm::NFC); });
})

NONIUS_BENCHMARK("ucd_nfc-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    meter.measure([&]() { unicode::normalized(data, unicode::NormalizationForm::NFD); });
})


/////
////


NONIUS_BENCHMARK("unilib_nfd-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    meter.measure([&]() { ufal::unilib::uninorms::nfc(str); });
})

NONIUS_BENCHMARK("unilib_nfd-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    meter.measure([&]() { ufal::unilib::uninorms::nfd(str); });
})

NONIUS_BENCHMARK("unilib_nfc-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    meter.measure([&]() { ufal::unilib::uninorms::nfc(str); });
})

NONIUS_BENCHMARK("unilib_nfc-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    meter.measure([&]() { ufal::unilib::uninorms::nfd(str); });
})


/////
/////
/////

template<typename NormalizationForm>
inline std::u32string ogonek_normalized(const std::u32string& in) {
    std::u32string str;
    str.reserve(in.size());
    auto r = ogonek::normalize<NormalizationForm>(in);
    unicode::copy(r, std::back_inserter(str));
    return str;
}

NONIUS_BENCHMARK("ogonek_nfd-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    meter.measure([&]() { ogonek_normalized<ogonek::nfc>(str); });
})

NONIUS_BENCHMARK("ogonek_nfd-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    meter.measure([&]() { ogonek_normalized<ogonek::nfd>(str); });
})

NONIUS_BENCHMARK("ogonek_nfc-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    meter.measure([&]() { ogonek_normalized<ogonek::nfc>(str); });
})

NONIUS_BENCHMARK("ogonek_nfc-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    meter.measure([&]() { ogonek_normalized<ogonek::nfd>(str); });
})


////
/// Qt
///
///
NONIUS_BENCHMARK("qt_nfd-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    QString qstr = QString::fromUcs4(str.data(), str.size());
    meter.measure([&]() { qstr.normalized(QString::NormalizationForm_C); });
})

NONIUS_BENCHMARK("qt_nfd-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFD);
    QString qstr = QString::fromUcs4(str.data(), str.size());
    meter.measure([&]() { qstr.normalized(QString::NormalizationForm_D); });
})

NONIUS_BENCHMARK("qt_nfc-nfc", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    QString qstr = QString::fromUcs4(str.data(), str.size());
    meter.measure([&]() { qstr.normalized(QString::NormalizationForm_C); });
})

NONIUS_BENCHMARK("qt_nfc-nfd", [](nonius::chronometer meter) {
    auto str = unicode::normalized(data, unicode::NormalizationForm::NFC);
    QString qstr = QString::fromUcs4(str.data(), str.size());
    meter.measure([&]() { qstr.normalized(QString::NormalizationForm_D); });
})


#endif

const std::u32string data = UR"(

                                    По оживлённым берегам
                                    Громады стройные теснятся
                                    Дворцов и башен; корабли
                                    Толпой со всех концов земли
                                    К богатым пристаням стремятся;
                                    Po oživlënnym beregam
                                    Gromady strojnye tesnâtsâ
                                    Dvorcov i bašen; korabli
                                    Tolpoj so vseh koncov zemli
                                    K bogatym pristanâm stremâtsâ;
                                    Ἰοὺ ἰού· τὰ πάντʼ ἂν ἐξήκοι σαφῆ.
                                    Ὦ φῶς, τελευταῖόν σε προσϐλέψαιμι νῦν,
                                    ὅστις πέφασμαι φύς τʼ ἀφʼ ὧν οὐ χρῆν, ξὺν οἷς τʼ
                                    οὐ χρῆν ὁμιλῶν, οὕς τέ μʼ οὐκ ἔδει κτανών
                                    Iou iou; ta pant' an exēkoi saphē.
                                    Ō phōs, teleutaion se prosblepsaimi nun,
                                    hostis pephasmai phus t' aph' hōn ou khrēn, xun hois t'
                                    ou khrēn homilōn, hous te m' ouk edei ktanōn.
                                    पशुपतिरपि तान्यहानि कृच्छ्राद्
                                    अगमयदद्रिसुतासमागमोत्कः ।
                                    कमपरमवशं न विप्रकुर्युर्
                                    विभुमपि तं यदमी स्पृशन्ति भावाः ॥
                                    Paśupatirapi tānyahāni kṛcchrād
                                    agamayadadrisutāsamāgamotkaḥ;
                                    kamaparamavaśaṃ na viprakuryur
                                    vibhumapi taṃ yadamī spṛśanti bhāvāḥ?
                                    子曰：「學而時習之，不亦說乎？有朋自遠方來，不亦樂乎？
                                    Zǐ yuē: “Xué ér shī xí zhī, bú yì yuè hū? Yoǔ péng zì yǔan fānglái, bú yì lè hū? Rén bù zhī, ér bú yùn, bú yì jūnzǐ hū?”
                                    ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்
                                    Sṟīṉivāsa Rāmāṉujaṉ Aiyaṅkār
                                    بِسْمِ ٱللّٰهِ ٱلرَّحْمـَبنِ ٱلرَّحِيمِ
                                    ٱلْحَمْدُ لِلّٰهِ رَبِّ ٱلْعَالَمِينَ
                                    ٱلرَّحْمـَبنِ ٱلرَّحِيمِ
                                    مَـالِكِ يَوْمِ ٱلدِّينِ
                                    إِيَّاكَ نَعْبُدُ وَإِيَّاكَ نَسْتَعِينُ
                                    ٱهْدِنَــــا ٱلصِّرَاطَ ٱلمُسْتَقِيمَ
                                    bismi ăl-la'hi ăr-raḥma'ni ăr-raḥiymi
                                    ăl-ḥamdu li-lla'hi rabbi ăl-`a'lamiyna
                                    ăr-raḥma'ni ăr-raḥiymi
                                    ma'liki yawmi ăd-diyni
                                    'iyya'ka na`budu wa-'iyya'ka nasta`iynu
                                    ĭhdina' ăṣ-ṣira'ṭa ăl-mustaqiyma
                                    Quizdeltagerne spiste jordbær med fløde, mens cirkusklovnen
                                    Wolther spillede på xylofon.
                                    (= Quiz contestants were eating strawbery with cream while Wolther
                                    the circus clown played on xylophone.)
                                    Falsches Üben von Xylophonmusik quält jeden größeren Zwerg
                                    (= Wrongful practicing of xylophone music tortures every larger dwarf)
                                    Zwölf Boxkämpfer jagten Eva quer über den Sylter Deich
                                    (= Twelve boxing fighters hunted Eva across the dike of Sylt)
                                    Heizölrückstoßabdämpfung
                                    (= fuel oil recoil absorber)
                                    (jqvwxy missing, but all non-ASCII letters in one word)
                                    Γαζέες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο
                                    (= No more shall I see acacias or myrtles in the golden clearing)
                                    Ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία
                                    (= I uncover the soul-destroying abhorrence)
                                    El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y
                                    frío, añoraba a su querido cachorro.
                                    (Contains every letter and every accent, but not every combination
                                    of vowel + acute.)
                                    Portez ce vieux whisky au juge blond qui fume sur son île intérieure, à
                                    côté de l'alcôve ovoïde, où les bûches se consument dans l'âtre, ce
                                    qui lui permet de penser à la cænogenèse de l'être dont il est question
                                    dans la cause ambiguë entendue à Moÿ, dans un capharnaüm qui,
                                    pense-t-il, diminue çà et là la qualité de son œuvre.
                                    l'île exiguë
                                    Où l'obèse jury mûr
                                    Fête l'haï volapük,
                                    Âne ex aéquo au whist,
                                    Ôtez ce vœu déçu.
                                    Le cœur déçu mais l'âme plutôt naïve, Louÿs rêva de crapaüter en
                                    canoë au delà des îles, près du mälström où brûlent les novæ.
                                    D'fhuascail Íosa, Úrmhac na hÓighe Beannaithe, pór Éava agus Ádhaimh
                                    Árvíztűrő tükörfúrógép
                                    (= flood-proof mirror-drilling machine, only all non-ASCII letters)
                                    Kæmi ný öxi hér ykist þjófum nú bæði víl og ádrepa
                                    Sævör grét áðan því úlpan var ónýt
                                    (some ASCII letters missing)
                                    Hiragana: (Iroha)
                                    いろはにほへとちりぬるを
                                    わかよたれそつねならむ
                                    うゐのおくやまけふこえて
                                    あさきゆめみしゑひもせす
                                    Katakana:
                                    イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム
                                    ウヰノオクヤマ ケフコエテ アサキユメミシ ヱヒモセスン
                                    ? דג סקרן שט בים מאוכזב ולפתע מצא לו חברה איך הקליטה
                                    Pchnąć w tę łódź jeża lub ośm skrzyń fig
                                    (= To push a hedgehog or eight bins of figs in this boat)
                                    В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!
                                    (= Would a citrus live in the bushes of south? Yes, but only a fake one!)
                                    Съешь же ещё этих мягких французских булок да выпей чаю
                                    (= Eat some more of these fresh French loafs and have some tea)
                                    ๏ เป็นมนุษย์สุดประเสริฐเลิศคุณค่า  กว่าบรรดาฝูงสัตว์เดรัจฉาน
                                    จงฝ่าฟันพัฒนาวิชาการ           อย่าล้างผลาญฤๅเข่นฆ่าบีฑาใคร
                                    ไม่ถือโทษโกรธแช่งซัดฮึดฮัดด่า     หัดอภัยเหมือนกีฬาอัชฌาสัย
                                    ปฏิบัติประพฤติกฎกำหนดใจ        พูดจาให้จ๊ะๆ จ๋าๆ น่าฟังเอย ฯ
                                    [The copyright for the Thai example is owned by The Computer
                                    Association of Thailand under the Royal Patronage of His Majesty the
                                    King.]
                                    Pijamalı hasta, yağız şoföre çabucak güvendi.
                                    (=Patient with pajamas, trusted swarthy driver quickly)
                                    عودة، محمد.
                                    كيف سقطت الملكية في مصر؟ :
                                    الطبعة 1.
                                    القاهرة :
                                    肖　显靜.
                                    第一　推动 /
                                    第1版.
                                    北京市 :
                                    吴　国林,
                                    芋生 裕信,
                                    西脇 順三郎 の 研究 :
                                    東京 :
                                    新典社 選書 ;
                                    澤 壽郎,
                                    片桐 一男,
                                    鎌倉市 中央 図書舘.
                                    نعيم، عبدالعزيز العلى.
                                    نظام الضرائب فى الإسلام ومدى تطبيقه فى المملكة العربية السعودية :
                                    回想 の 西脇 順三郎 /
                                    東京 :
                                    西脇 順三郎,
                                    安東 伸介,
                                    西脇 順三郎,
                                    イギリス 文學史 :
                                    東京 :
                                    英語 英文學 講座
                                    تهران
                                    صفا، ذبيح الله.
                                    金田 弘,
                                    旅人 つひに かへらず :
                                    東京 :
                                    西脇 順三郎,
                                    西脇 順三郎,
                                    幻影 の 人 西脇 順三郎 を 語る /
                                    西脇 順三郎 を 語る
                                    第 1版.
                                    東京 :
                                    西脇 順三郎,
                                    西脇 順三郎 を 偲ぶ 会.
                                    双鸭山　林业局 志 /
                                    第1版.
                                    [双鸭山? :
                                    黑龙江省 (China).
                                    伊 蝉.
                                    非常 误会/
                                    Singapore :
                                    狮城 作家 群英 丛书 ;
                                    فرزاد، عبد الحسين.
                                    المنهج في تاريخ الأدب العربي :
                                    Tehran :
                                    夏 枯早.
                                    涼風 有信 /
                                    Kuala Lumpur :
                                    丹袖 出版 系列 ;
                                    子木,
                                    杭州 日记 /
                                    [Singapore] :
                                    子木,
                                    جائزة سلطان العويس الثقافية :
                                    الشارقة :
                                    منشورات اتحاد كتاب وادباء الامارات
                                    مؤسسة سلطان بن علي العويس الثقافية.
                                    اتحاد كتاب وأدباء الإمارات.
                                    旱区　水　土　作物　关系　及　其　最优　调控　原理 :
                                    第1版.
                                    北京 :
                                    广西 壮族 自治区 南宁市 国家 稅务局.
                                    广西 壮族 自治区 南宁市 地方 稅务局.
                                    中国　副食品　市场　需求　与　"菜篮子　工程" 布局 /
                                    第1版.
                                    北京 :
                                    رياض، محمد.
                                    الروح الايماني في الشعر العربي :
                                    الطبعة 1.
                                    بغداد :
                                    نفحات الهند واليمن بأسانيد الشيخ أبي الحسن :
                                    الطبعة 1.
                                    الرياض :
                                    قاضي، النعمان.
                                    شعر الفتوح الإسلامية في صدر الإسلام /
                                    الطبعة 1.
                                    جدة :
                                    من أدب الجهاد
                                    العلاقات المصرية-اليابانية /
                                    الجيزة :
                                    مدني، إسماعيل محمد.
                                    بيئة البحرين البحرية /
                                    البحرين :
                                    سلسلة كتب حول الحياة الفطرية ؛
                                    실크  로드　와　韓國　文化　의　探究　/
                                    대전　광역시 :
                                    人文　硏究　學術　叢書 ;
                                    史　在東,
                                    红盾　耕耘录 :
                                    第1版.
                                    [厦门] :
                                    龙海市 (China)
                                    بدر، عزة.
                                    أم الدنيا :
                                    كتاب الجمهورية
                                    خلايلي، خليل.
                                    تاريخ جسكالا :
                                    الطبعة 1.
                                    دمشق ؛
                                    راوى، صلاح.
                                    فلسفة الوعى الشعبى :
                                    الطبعة 1.
                                    القاهرة :
                                    刘  宏.
                                    百年  梦寻 :
                                    20世纪  中国  经济  思潮  与  社会  变革
                                    第1版.
                                    北京市 :
                                    世纪  回眸.
                                    沈  山.
                                    石  淑华,
                                    趋势 与 策略 /
                                    Kuala Lumpur :
                                    林 水〓.
                                    陈 友信.
                                    西脇 順三郎,
                                    西脇 順三郎・パイオニア の 仕事 /
                                    パイオニア の 仕事
                                    第 1版.
                                    東京 :
                                    コレクション・日本 シュ－ルレアリスム ;
                                    和田 桂子,
                                    申 國美.
                                    1900-2001 國家　圖書館  藏　敦煌　遺書　研究　論著　目錄　索引 /
                                    敦煌　遺書　研究　論著　目錄　索引
                                    國家　圖書館  藏　敦煌　遺書　研究　論著　目錄　索引
                                    第1版.
                                    北京市 :
                                    广州 :
                                    发现 之旅 =
                                    徽州 地区(China)
                                    徽州 地区(China)
                                    南 丽军.
                                    比较 政治 制度 /
                                    第1版.
                                    哈尔滨 :
                                    向 俊杰.
                                    孔 兆政.
                                    贾 少华.
                                    民办 大学 的 战略 /
                                    第1版.
                                    杭州 :
                                    浙江省 教育 科学 规划 2005 年 重点 研究 课题
                                    刘 晓琴.
                                    中国 近代 留英 教育史 /
                                    第1版.
                                    天津市 :
                                    近代 中国 研究 丛书
                                    Rev. ed. of author's doctoral thesis under title: 中国 近代 留英 教育 研究.
                                    刘 晓琴.
                                    党 崇民,
                                    论 邓小平 战役 指导 特色 =
                                    第1版.
                                    北京 :
                                    中国 军事学 博士 文库
                                    邓 小平,
                                    建立 农村 劳动力 平等 就业 制度 /
                                    第1版.
                                    北京市 :
                                    陈 晓华.
                                    张 红宇.
                                    胡 小林.
                                    毛 泽东 的 学习 思想 与 实践 /
                                    第1版.
                                    济南 :
                                    毛 泽东,
                                    于 云才.
                                    香港 :
                                    北京 大學 華僑 華人 研究 中心 叢書 ;
                                    Translation of: 落地 生根 : 神戶 華僑 と 神阪 中華 会館 の 百年.
                                    中華 会館 (Kobe-shi, Japan)
                                    忽 海燕.
                                    中華 會館 (Kobe-shi, Japan)
                                    王 鼎鈞.
                                    葡萄 熟了 /
                                    1版.
                                    臺北市 :
                                    大地 叢書 ;
                                    戴 錦華,
                                    性別 中國 /
                                    初版.
                                    台北市 :
                                    麥田 人文
                                    附 參考 文獻.
                                    盧 增.
                                    柬埔寨 企业 纳稅 指南, 2004 /
                                    中文 版.
                                    [Phnom Penh?] :
                                    הלר חדד, הילה.
                                    התכלית של ועדות מייעצות ציבוריות לעניין מסוים־ נוסח אד הוקת בישראל : תוך התמקדות בשלוש הועדות הבאות: ועדת וילנאי (1999), ועדת טל (2000), ועדת בן בסט (2000) /
                                    [Israel :
                                    פרג׳, רג׳א סעיד.
                                    הקשרים בין הדרוזים והיהודים עד הקמת מדינת ישראל (1948) /
                                    ינוח :
                                    חלפין, יגאל.
                                    הטיהורים הסטליניסטיים :
                                    תל אביב :
                                    פטיש
                                    קרליץ, שמריהו יוסף נסים.
                                    חוט שני (הלכות רבית)
                                    ספר חוט שני :
                                    בני ברק :
                                    הוכמן, חיים אריה,
                                    안 도섭.
                                    조선 의 혼불 타던 밤 에 :
                                    초판.
                                    서울시 :
                                    윤 동주,
                                    بلفقيه، عبد الله بن حسين بن عبد الله.
                                    قوت الألباب من مجاني جني الألباب /
                                    تريم، حضرموت :
                                    زكي، رمضان خميس.
                                    مفهوم السنن الربانية :
                                    الطبعة 1.
                                    القاهرة :
                                    هذا هو الإسلام ؛
                                    عمارة، محمد.
                                    قراءة النص الديني :
                                    الطبعة 1.
                                    القاهرة :
                                    هذا هو الإسلام ؛
                                    ندوة السيرة النبوية :
                                    الطبعة 1.
                                    [Omdurman] :
                                    منشورات المركز ؛
                                    إمام، حمادة.
                                    مبارك والإخوان /
                                    الطبعة العربية 1.
                                    المهندسين [Giza] :
                                    新譯 古文 辭類纂 /
                                    初版.
                                    臺北市 :
                                    古籍 今注 新譯 叢書
                                    姚 鼐,
                                    黃 鈞.
                                    刘 文锁,
                                    尼雅 :
                                    貴志 俊彦,
                                    川島 真,
                                    בסן, צדוק.
                                    יתומות :
                                    ירושלים :
                                    רז, גאי.
                                    בית האמנים.
                                    שלו, אריה.
                                    כישלון והצלחה בהתרעה :
                                    תל אביב :
                                    אריאלי, שאול.
                                    תפסת מרובה לא תפסת :
                                    ירושלים :
                                    תמונת מצב
                                    שביט, יעקב.
                                    אירופה המהוללת והמקוללת :
                                    תל אביב :
                                    ריינהרץ, יהודה.
                                    שי, דב.
                                    15 מכל 100 שרדו :
                                    חמישה עשר מכל מאה שרדו
                                    מהד׳ 2.
                                    [ת״א ז״א תל אביב :
                                    שי, דב.
                                    עדווי, ג׳מאל,
                                    פעילות הקוויקרים האמריקנים בפלישתינה 1948-1869 /
                                    Haifa] :
                                    חסקין, גילי.
                                    שבוי בקסמה :
                                    הוד השרון :
                                    טאוב, גדי.
                                    המתנחילים והמאבק על משמעותה של הציונות /
                                    מתנחלים
                                    תל אביב :
                                    פרוזה (ידיעות אחרונות).
                                    日本軍 「慰安婦」 関係 資料 集成 /
                                    初版.
                                    東京 :
                                    鈴木 裕子,
                                    山下 英愛,
                                     外村 大,1966-
                                    نجمآبادي، أبو الفضل.
                                    كتاب القضاء :
                                    الطبعة 1.
                                    قم :
                                    مجموعة ىثار آية الله الميرزا أبو الفضل النجم آبادي ؛
                                    نجمآبادي، أبو الفضل.
                                    الرسائل الفقهية :
                                    الطبعة 1.
                                    قم :
                                    مجموعة آثار آية الله الميرزا أبو الفضل النجمآبادي ؛
                                    한국 의 민주 정부.
                                    한국 의 민주 정부.
                                    חיים ביהודה.
                                    [Jerusalem] :
                                    松村 潤,
                                    米國 議會 圖書館 所藏 滿洲語 文獻 目錄 /
                                    滿洲語 文獻 目錄
                                    東京 :
                                    東北 アジア 文獻 硏究 叢刊 ;
                                    פוגל, דוד.
                                    חרא של פרסום /
                                    מהד׳ 1.
                                    [Israel] :
                                    פוגל, דוד.
                                    ישראל.
                                    אזורי עדיפות לאומית :
                                    ירושלים :
                                    دارا شكوه.
                                    منتخبات آثار :
                                    [تهران] :
                                    آيتى، ابراهيم.
                                    غيرت كرمانشاهى، عبد الكريم.
                                    كليات آثار سيد عبد الكريم غيرت كرمانشاهى /
                                    [تهران :
                                    غيرت، محمد سعيد.
)";
