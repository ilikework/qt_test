#!/usr/bin/env python3
"""Find ja translations that still look like Chinese."""
from __future__ import annotations

from pathlib import Path

GEN = Path(__file__).resolve().parent / "gen_qm.py"
text = GEN.read_text(encoding="utf-8")
start = text.index("TABLE:")
end = text.index("\nLOCALES", start)
ns: dict = {}
exec(compile(text[start:end], "gen_qm.py", "exec"), ns)
TABLE: dict[str, dict[str, str]] = ns["TABLE"]

# Simplified-only / mainland wording in ja
SIMPLIFIED_CHARS = set("国语体会头发发现贝车东丝两严丧个丰临为丽举义乌乐乔习乡书买乱争于亏云亚产亩亲亿仅从仓仪们价众优会伞伟传伤伦伪伫体余佣佥侠侣侥侦侧侨侩侪侬俣俦俨俩俪俭债倾偬偻偾偿傥傧储傩儿兑兖党兰关兴兹养兽冁内冈册写军农冯况冻净凄准凉凌减凑凛几凤凫凭凯击凿刍划刘则刚创删别刬刭刽刿剀剂剐剑剥剧劝办务劢动励劲劳势勋勐勚匀匦匮区医华协单卖卢卤卧卫却卵厂厅历厉压厌厍厕厘厚厝原厢厣厥厦厨厩厮县叁参又双发变叙叠叶号叹叽吁后吓吕吗吖吨听启吴吵吸吹吻吼吾呀呃呆呈呐呓呕呓呗员呛呜呢呤呦周呱呲味呵呷呸呻呼命咀咂咄咆咋和咎咏咐咒咔咕咖咙咧咨咪咫咬咭咯咱咳咴咸咻咽咿哀品哂哄哆哇哈哉哌响哎哏哐哑哒哓哔哕哗哙哚哜哝哞哟哥哦哧哨哩哪哭哮哲哺哼哽哿唁唆唇唉唏唐唑唔唛唠唢唤唧唬售唯唰唱唳唷唾啃啄商啉啊啐啕啖啜啡啤啥啦啧啪啬啭啮啰啴啸啼喀喁喂喃善喇喈喉喊喋喏喑喔喘喙喜喝喟喧喱喳喵喷喹喻喽喾嗄嗅嗉嗌嗍嗑嗒嗓嗔嗖嗜嗝嗡嗤嗥嗦嗨嗪嗫嗬嗯嗲嗳嗵嗷嗽嗾嘀嘁嘈嘉嘌嘎嘏嘘嘛嘞嘟嘣嘤嘧嘬嘭嘱嘲嘴嘶嘹嘻嘿噌噍噎噔噗噘噙噜噢噤器噩噪噫噬噱噶噻噼嚅嚆嚎嚏嚓嚣嚯嚷嚼囊囔囚四回因团囤囫园困囱围囵国图圆圣在圩圪圬圭圮圯地场圾址坂均坊坌坍坎坏坐坑块坚坛坜坝坞坟坠坡坤坦坪坭坩坯坳坶坷坻坼垂垃垄垅垆型垒垓垛垡垢垣垤垦垧垩垫垭垮垲垴垵垸埂埃埋城埏埔埕埘埙埚埝域埠埤埭埯埴埸培基埽堂堃堆堇堉堡堤堠堨堪堰堵堽堿塄塅塆塌塍塑塔塘塞塥填塬塾墀墁境墅墉墒墓墙增墟墦墨墩墼壁壅壑壕壤士壬壮声壳壶壹处备复夏夕外夙多夜够夤夥大天太夫夭央夯失头夷夸夹夺夼奁奂奄奇奈奉奋奏契奔奕奖套奘奚奠奢奥女奴奶奸她好如妃妄妆妇妈妒妓妖妙妗妞妣妤妥妨妩妪妫姗妮妲妹妻妾姆姊始姐姑姒姓委姗姚姜姝姣姥姨姬姻姿威娃娄娅娆娇娈娉娌娑娓娘娜娟娠娣娥娩娱娲娴娶娼婀婆婉婊婕婚婢婧婪婴婵婶婷婺婿媒媚媛媪媲媳媵媸媾嫁嫂嫉嫌嫒嫔嫖嫘嫜嫠嫡嫣嫦嫩嫪嫫嫱嬉嬖嬗嬛嬴嬷孀子孑孔孕字存孙孚孛孜孝孟孢季孤学孩孪孬孰孱孳孵孺孽宁它宄宅宇守安宋完宏宓宕宗官宙定宛宜宝实宠审客宣室宥宦宪宫宰害宴宵家宸容宽宾宿寂寄寅密寇富寐寒寓寝寞察寡寥寨寮寰寸对寺寻导寿封射将尉尊小少尔尖尘尚尝尤尧尬就尴尸尹尺尼尽尾尿局屁层屃居屈屉届屋屎屏屐屑展屙属屠屡屣履屦屯山屹屿岁岂岈岌岐岑岔岖岗岘岙岚岛岩岜岍岐岑岔岖岗岘岙岚岛岩岜岍岐岑岔岖岗岘岙岚岛岩岜岍")

CHINESE_TERMS = ("定位", "客户", "肌肤", "褐色斑", "综合", "资料", "预录", "备份", "恢复", "删除", "确认", "设置", "检测", "报告", "性别", "电话", "邮箱", "生日", "登记")

issues: list[tuple[str, str, str]] = []
for k, row in sorted(TABLE.items()):
    ja = row.get("ja", "")
    zh = row.get("zh_CN", "")
    if ja == zh and len(zh) > 1 and any("\u4e00" <= c <= "\u9fff" for c in zh):
        issues.append((k, ja, "ja==zh_CN"))
    for term in CHINESE_TERMS:
        if term in ja and term not in ("定位",):  # 定位 handled separately
            issues.append((k, ja, f"term:{term}"))
            break
    if "定位" in ja:
        issues.append((k, ja, "term:定位"))
    if any(c in SIMPLIFIED_CHARS for c in ja):
        issues.append((k, ja, "simplified_char"))

# dedupe
seen = set()
unique = []
for item in issues:
    if item[0] not in seen:
        seen.add(item[0])
        unique.append(item)

out = Path(__file__).resolve().parent / "_ja_issues.txt"
with out.open("w", encoding="utf-8") as f:
    f.write(f"issues: {len(unique)}\n\n")
    for k, ja, reason in unique:
        f.write(f"[{reason}] {k!r}\n  -> {ja!r}\n\n")
print(f"wrote {out} ({len(unique)} issues)")
