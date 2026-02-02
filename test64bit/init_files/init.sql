--1,
CREATE TABLE IF NOT EXISTS T_VersionInfo ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , VERSION INTEGER
  , COMMENT TEXT
);

--2,
CREATE TABLE IF NOT EXISTS T_CameraConfig( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Camera TEXT
  , Series INTEGER
  , ConfigType TEXT
  , ConfigText TEXT
  , ConfigValue INTEGER
  , OrderID INTEGER
);

--3,
CREATE TABLE IF NOT EXISTS T_CaptureSetting ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Series INTEGER
  , ImageSize INTEGER
  , ImageQuality INTEGER
  , RGB_ISO INTEGER
  , RGB_ExposureTime INTEGER
  , RGB_Aperture INTEGER
  , RGB_WB INTEGER
  , UV_ISO INTEGER
  , UV_ExposureTime INTEGER
  , UV_Aperture INTEGER
  , UV_WB INTEGER
  , PL_ISO INTEGER
  , PL_ExposureTime INTEGER
  , PL_Aperture INTEGER
  , PL_WB INTEGER
  , NPL_ISO INTEGER
  , NPL_ExposureTime INTEGER
  , NPL_Aperture INTEGER
  , NPL_WB INTEGER
  , Gender INTEGER -- 1:Male,2:Female
);

--4,
CREATE TABLE IF NOT EXISTS T_Offering ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Type  INTEGER   -- 1:Product,2:Service
  , Name  TEXT
  , Price REAL
  , Des TEXT
);

--5,
CREATE TABLE IF NOT EXISTS T_Customers( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Cust_ID TEXT
  , Cust_Name TEXT
  , Cust_Gender INTEGER-- 1:Male,2:Female
  , Cust_Birthday TEXT
  , Cust_EditTime TEXT
  , Cust_Phone TEXT
  , Cust_Addr TEXT
  , Cust_EMail TEXT
  , Cust_Photo TEXT
  , Cust_Height TEXT
  , Cust_Weight TEXT
  , Cust_Des TEXT
);

--6,
CREATE TABLE IF NOT EXISTS T_Customers_FacePhoto (
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Cust_ID TEXT
  , Photo_CapType TEXT  -- RGB/UV/PL/NPL/GRAY/RED/BROWN/WHOLE
  , Photo_DirType TEXT  -- L/R/M 
  , Group_ID INTEGER
  , Photo_ID INTEGER
  , Photo_Name TEXT
  , Photo_EditTime DATETIME
  , photo_iso TEXT
  , photo_wb TEXT
  , photo_aperture TEXT
  , photo_exposuretime TEXT
  , photo_comment TEXT
);

--7,
CREATE TABLE T_FacePhoto_DrawInfo_Template(
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Photo_DirType TEXT                          -- _L,_R,_M
  , Info TEXT                                     -- json for the photo draw included draw type(circle/rectangle/line/Selection Box),points array,picture size,line weight
  , EditTime DATETIME
);

--8,
CREATE TABLE IF NOT EXISTS T_FacePhoto_DrawInfo ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , FacePhoto_IX INTEGER   -- -1 for default DrawInfo like Left/Right Selection Box
  , Info TEXT  -- json for the photo draw included draw type(circle/rectangle/line/Selection Box),points array,picture size,line weight
  , EditTime DATETIME
);

--9,
CREATE TABLE IF NOT EXISTS T_FacePhoto_AnalyseInfo ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , FacePhoto_IX INTEGER
  , Analyse_Function INTEGER -- 1:AnalyseSpots-PL/NPL/BROWN,2:AnalysePores-RGB;3:AnalyseEvenness-RED;4:AnalyseWrinkle-GRAY;5:AnalyseAcnes-UV;21:Moisture-WHOLE
  , Analyse_Result INTEGER
  , Analyse_Precent INTEGER
  , EditTime DATETIME
);

--10,
CREATE TABLE IF NOT EXISTS T_FacePhoto_Map ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Photo_CapType    TEXT     -- RGB/UV/PL/NPL/GRAY/RED/BROWN/WHOLE
  , Analyse_Function INTEGER  -- 1:AnalyseSpots-PL/NPL/BROWN,2:AnalysePores-RGB;3:AnalyseEvenness-RED;4:AnalyseWrinkle-GRAY;5:AnalyseAcnes-UV;21:Moisture-WHOLE
  , Report_Type      INTEGER  -- 1:RGB-Pore;2:UV-Acne;3:PL-DeepSpots;4:NPL-SurfaceSpot;5:GRAY-Wrinkle;6:RED-Sensitivity;7:BROWN-DarkSpot;8:WHOLE-Moisture
);

--11,
CREATE TABLE IF NOT EXISTS T_Report_Template ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Report_Type  INTEGER   -- 1:RGB-Pore;2:UV-Acne;3:PL-DeepSpots;4:NPL-SurfaceSpot;5:GRAY-Wrinkle;6:RED-Sensitivity;7:BROWN-DarkSpot;8:WHOLE-Moisture;100:Total
  , Report_LEVEL INTEGER   -- 10:Bad,20:Normal,30:Good
  , MEMO TEXT
);

--12,
CREATE TABLE IF NOT EXISTS T_Report_Offerings_Template ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Report_IX    INTEGER
  , Offering_IX  INTEGER
);

--13,
CREATE TABLE IF NOT EXISTS T_Report_Main ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Report_Type  INTEGER   -- 1:RGB-Pore;2:UV-Acne;3:PL-DeepSpots;4:NPL-SurfaceSpot;5:GRAY-Wrinkle;6:RED-Sensitivity;7:BROWN-DarkSpot;8:WHOLE-Moisture;100:Total
  , Report_LEVEL INTEGER   -- 10:Bad,20:Normal,30:Good
  , MEMO TEXT
  , EditTime DATETIME
);

--14,
CREATE TABLE IF NOT EXISTS T_Report_Offering ( 
  IX INTEGER PRIMARY KEY AUTOINCREMENT
  , Report_IX  INTEGER
  , Offering_IX  INTEGER
);


insert into T_VersionInfo(IX,VERSION,[COMMENT]) values (1,100,'第一个被管理的版本，希望能够被以后的版本兼容');
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (1,'G7',1,'wb','Daylight',1,44);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (2,'G7',1,'wb','Cloudy',2,45);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (3,'G7',1,'wb','Tungsten',3,46);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (4,'G7',1,'wb','Fluorescent',4,47);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (5,'G7',1,'wb','Flash',5,48);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (6,'G7',1,'iso','80',69,49);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (7,'G7',1,'iso','100',72,50);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (8,'G7',1,'iso','200',80,51);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (9,'G7',1,'iso','400',88,52);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (10,'G7',1,'iso','800',96,53);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (11,'G7',1,'iso','1600',104,54);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (12,'G7',1,'ImageSize','Large',0,55);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (13,'G7',1,'ImageSize','Medium1',1,56);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (14,'G7',1,'ImageSize','Small',2,57);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (15,'G7',1,'ImageSize','Medium1',3,58);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (16,'G7',1,'ImageSize','Medium2',7,59);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (17,'G7',1,'ImageQuality','Normal',2,61);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (18,'G7',1,'ImageQuality','Fine',3,62);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (19,'G7',1,'ImageQuality','SuperFine',5,64);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (20,'G7',1,'aperture','2.8',32,65);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (21,'G7',1,'aperture','3.2',35,66);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (22,'G7',1,'aperture','3.5(1/3)',37,67);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (23,'G7',1,'aperture','4.0',40,68);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (24,'G7',1,'aperture','4.5(1/3)',43,69);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (25,'G7',1,'aperture','5.6(1/3)',45,70);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (26,'G7',1,'aperture','5.6',48,71);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (27,'G7',1,'aperture','6.3',51,72);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (28,'G7',1,'aperture','7.1',53,73);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (29,'G7',1,'aperture','8.0',56,74);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (30,'G7',1,'exposuretime','15"',24,75);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (31,'G7',1,'exposuretime','13"',27,76);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (32,'G7',1,'exposuretime','10"',28,77);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (33,'G7',1,'exposuretime','8"',32,78);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (34,'G7',1,'exposuretime','6"(1/3)',35,79);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (35,'G7',1,'exposuretime','5"',37,80);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (36,'G7',1,'exposuretime','4"',40,81);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (37,'G7',1,'exposuretime','3"2',43,82);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (38,'G7',1,'exposuretime','2"5',45,83);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (39,'G7',1,'exposuretime','2"',48,84);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (40,'G7',1,'exposuretime','1"6',51,85);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (41,'G7',1,'exposuretime','1"3',53,86);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (42,'G7',1,'exposuretime','1"',56,87);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (43,'G7',1,'exposuretime','0"8',59,88);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (44,'G7',1,'exposuretime','0"6',61,89);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (45,'G7',1,'exposuretime','0"5',64,90);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (46,'G7',1,'exposuretime','0"4',67,91);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (47,'G7',1,'exposuretime','0"3(1/3)',69,92);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (48,'G7',1,'exposuretime','1/4',72,93);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (49,'G7',1,'exposuretime','1/5',75,94);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (50,'G7',1,'exposuretime','1/6(1/3)',77,95);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (51,'G7',1,'exposuretime','1/8',80,96);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (52,'G7',1,'exposuretime','1/10(1/3)',83,97);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (53,'G7',1,'exposuretime','1/13',85,98);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (54,'G7',1,'exposuretime','1/15',88,99);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (55,'G7',1,'exposuretime','1/20(1/3)',91,100);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (56,'G7',1,'exposuretime','1/25',93,101);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (57,'G7',1,'exposuretime','1/30',96,102);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (58,'G7',1,'exposuretime','1/40',99,103);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (59,'G7',1,'exposuretime','1/50',101,104);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (60,'G7',1,'exposuretime','1/60',104,105);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (61,'G7',1,'exposuretime','1/80',107,106);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (62,'G7',1,'exposuretime','1/100',109,107);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (63,'G7',1,'exposuretime','1/125',112,108);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (64,'G7',1,'exposuretime','1/160',115,109);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (65,'G7',1,'exposuretime','1/200',117,110);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (66,'G7',1,'exposuretime','1/250',120,111);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (67,'G7',1,'exposuretime','1/320',123,112);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (68,'G7',1,'exposuretime','1/400',125,113);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (69,'G7',1,'exposuretime','1/500',128,114);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (70,'G7',1,'exposuretime','1/640',131,115);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (71,'G7',1,'exposuretime','1/800',133,116);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (72,'G7',1,'exposuretime','1/1000',136,117);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (73,'G7',1,'exposuretime','1/1250',139,118);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (74,'G7',1,'exposuretime','1/1600',141,119);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (75,'Aigo',5,'wb','Auto',0,120);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (76,'Aigo',5,'wb','Daylight',1,121);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (77,'Aigo',5,'wb','Cloudy',2,122);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (78,'Aigo',5,'wb','filament lamp',3,123);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (79,'Aigo',5,'wb','Fluorescent1',4,124);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (80,'Aigo',5,'wb','Fluorescent2',5,125);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (81,'Aigo',5,'wb','custom',6,126);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (82,'Aigo',5,'iso','Auto',0,127);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (83,'Aigo',5,'iso','100',1,128);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (84,'Aigo',5,'iso','200',2,129);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (85,'Aigo',5,'iso','400',3,130);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (86,'Aigo',5,'iso','800',4,131);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (87,'Aigo',5,'iso','1600',5,132);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (88,'Aigo',5,'iso','3200',6,133);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (89,'Aigo',5,'iso','6400',7,134);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (90,'Aigo',5,'ImageQuality','Normal',2,135);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (91,'Aigo',5,'ImageQuality','Fine',1,136);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (92,'Aigo',5,'ImageQuality','SuperFine',0,137);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (93,'Aigo',5,'ImageSize','640x480',0,138);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (94,'Aigo',5,'ImageSize','1024x768',1,139);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (95,'Aigo',5,'ImageSize','1600x1200',2,140);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (96,'Aigo',5,'ImageSize','3M',3,141);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (97,'Aigo',5,'ImageSize','4M',4,142);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (98,'Aigo',5,'ImageSize','5M',5,143);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (99,'Aigo',5,'ImageSize','6M',6,144);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (100,'Aigo',5,'ImageSize','8M',8,145);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (101,'Aigo',5,'ImageSize','10M',10,146);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (102,'Aigo',5,'ImageSize','12M',12,147);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (103,'Aigo',5,'ImageSize','14M',14,148);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (104,'Aigo',5,'aperture','Big',0,149);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (105,'Aigo',5,'aperture','Small',1,150);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (106,'Aigo',5,'exposuretime','15"',0,151);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (107,'Aigo',5,'exposuretime','13"',1,152);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (108,'Aigo',5,'exposuretime','10"',2,153);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (109,'Aigo',5,'exposuretime','8"',3,154);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (110,'Aigo',5,'exposuretime','6"',4,155);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (111,'Aigo',5,'exposuretime','5"',5,156);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (112,'Aigo',5,'exposuretime','4"',6,157);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (113,'Aigo',5,'exposuretime','3"2',7,158);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (114,'Aigo',5,'exposuretime','2"5',8,159);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (115,'Aigo',5,'exposuretime','2"',9,160);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (116,'Aigo',5,'exposuretime','1"6',10,161);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (117,'Aigo',5,'exposuretime','1"3',11,162);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (118,'Aigo',5,'exposuretime','1"',12,163);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (119,'Aigo',5,'exposuretime','0"8',13,164);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (120,'Aigo',5,'exposuretime','0"6',14,165);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (121,'Aigo',5,'exposuretime','0"5',15,166);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (122,'Aigo',5,'exposuretime','0"4',16,167);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (123,'Aigo',5,'exposuretime','0"3',17,168);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (124,'Aigo',5,'exposuretime','1/4',18,169);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (125,'Aigo',5,'exposuretime','1/5',19,170);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (126,'Aigo',5,'exposuretime','1/6',20,171);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (127,'Aigo',5,'exposuretime','1/8',21,172);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (128,'Aigo',5,'exposuretime','1/10',22,173);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (129,'Aigo',5,'exposuretime','1/13',23,174);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (130,'Aigo',5,'exposuretime','1/15',24,175);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (131,'Aigo',5,'exposuretime','1/20',25,176);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (132,'Aigo',5,'exposuretime','1/25',26,177);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (133,'Aigo',5,'exposuretime','1/30',27,178);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (134,'Aigo',5,'exposuretime','1/40',28,179);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (135,'Aigo',5,'exposuretime','1/50',29,180);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (136,'Aigo',5,'exposuretime','1/60',30,181);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (137,'Aigo',5,'exposuretime','1/80',31,182);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (138,'Aigo',5,'exposuretime','1/100',32,183);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (139,'Aigo',5,'exposuretime','1/125',33,184);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (140,'Aigo',5,'exposuretime','1/160',34,185);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (141,'Aigo',5,'exposuretime','1/200',35,186);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (142,'Aigo',5,'exposuretime','1/250',36,187);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (143,'Aigo',5,'exposuretime','1/320',37,188);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (144,'Aigo',5,'exposuretime','1/400',38,189);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (145,'Aigo',5,'exposuretime','1/500',39,190);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (146,'Aigo',5,'exposuretime','1/640',40,191);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (147,'Aigo',5,'exposuretime','1/800',41,192);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (148,'Aigo',5,'exposuretime','1/1000',42,193);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (149,'Aigo',5,'exposuretime','1/1250',43,194);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (150,'Aigo',5,'exposuretime','1/1600',44,195);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (151,'Aigo',5,'exposuretime','1/2000',45,196);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (152,'Aigo',5,'flashmode','On',1,197);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (153,'Aigo',5,'flashmode','Off',0,198);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (154,'Aigo',5,'ImageSize','20M',20,251);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (155,'CanonEOS',4,'wb','Auto',0,301);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (156,'CanonEOS',4,'wb','Daylight',1,302);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (157,'CanonEOS',4,'wb','Cloudy',2,303);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (158,'CanonEOS',4,'wb','Tungsten',3,304);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (159,'CanonEOS',4,'wb','Fluorescent',4,305);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (160,'CanonEOS',4,'wb','Flash',5,306);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (161,'CanonEOS',4,'iso','Auto',0,307);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (162,'CanonEOS',4,'iso','50',64,308);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (163,'CanonEOS',4,'iso','100',72,309);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (164,'CanonEOS',4,'iso','125',75,310);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (165,'CanonEOS',4,'iso','160',77,311);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (166,'CanonEOS',4,'iso','200',80,312);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (167,'CanonEOS',4,'iso','250',83,313);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (168,'CanonEOS',4,'iso','320',85,314);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (169,'CanonEOS',4,'iso','400',88,315);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (170,'CanonEOS',4,'iso','500',91,316);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (171,'CanonEOS',4,'iso','640',93,317);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (172,'CanonEOS',4,'iso','800',96,318);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (173,'CanonEOS',4,'iso','1000',99,319);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (174,'CanonEOS',4,'iso','1250',101,320);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (175,'CanonEOS',4,'iso','1600',104,321);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (176,'CanonEOS',4,'iso','2000',107,322);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (177,'CanonEOS',4,'iso','2500',109,323);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (178,'CanonEOS',4,'iso','3200',112,324);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (179,'CanonEOS',4,'iso','3200',112,325);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (180,'CanonEOS',4,'iso','4000',115,326);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (181,'CanonEOS',4,'iso','5000',117,327);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (182,'CanonEOS',4,'iso','6400',120,328);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (183,'CanonEOS',4,'iso','8000',123,329);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (184,'CanonEOS',4,'iso','10000',125,330);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (185,'CanonEOS',4,'iso','12800',128,331);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (186,'CanonEOS',4,'iso','16000',131,332);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (187,'CanonEOS',4,'iso','20000',133,333);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (188,'CanonEOS',4,'iso','25600',136,334);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (189,'CanonEOS',4,'ImageSize','Large',0,335);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (190,'CanonEOS',4,'ImageQuality','Fine',3,339);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (191,'CanonEOS',4,'aperture','2.8',32,340);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (192,'CanonEOS',4,'aperture','3.2',35,341);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (193,'CanonEOS',4,'aperture','3.5',36,342);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (194,'CanonEOS',4,'aperture','3.5(1/3)',37,343);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (195,'CanonEOS',4,'aperture','4.0',40,344);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (196,'CanonEOS',4,'aperture','4.5(1/3)',43,345);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (197,'CanonEOS',4,'aperture','5',45,346);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (198,'CanonEOS',4,'aperture','5.6',48,347);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (199,'CanonEOS',4,'aperture','6.3',51,348);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (200,'CanonEOS',4,'aperture','7.1',53,349);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (201,'CanonEOS',4,'aperture','8.0',56,350);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (202,'CanonEOS',4,'aperture','9',59,351);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (203,'CanonEOS',4,'aperture','9.5',60,352);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (204,'CanonEOS',4,'aperture','11',64,353);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (205,'CanonEOS',4,'aperture','13',68,354);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (206,'CanonEOS',4,'aperture','14',69,355);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (207,'CanonEOS',4,'aperture','16',72,356);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (208,'CanonEOS',4,'aperture','18',75,357);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (209,'CanonEOS',4,'aperture','20',77,358);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (210,'CanonEOS',4,'aperture','22',80,359);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (211,'CanonEOS',4,'exposuretime','15"',24,360);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (212,'CanonEOS',4,'exposuretime','13"',27,361);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (213,'CanonEOS',4,'exposuretime','10"',28,362);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (214,'CanonEOS',4,'exposuretime','8"',32,363);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (215,'CanonEOS',4,'exposuretime','6"(1/3)',35,364);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (216,'CanonEOS',4,'exposuretime','5"',37,365);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (217,'CanonEOS',4,'exposuretime','4"',40,366);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (218,'CanonEOS',4,'exposuretime','3"2',43,367);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (219,'CanonEOS',4,'exposuretime','2"5',45,368);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (220,'CanonEOS',4,'exposuretime','2"',48,369);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (221,'CanonEOS',4,'exposuretime','1"6',51,370);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (222,'CanonEOS',4,'exposuretime','1"3',53,371);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (223,'CanonEOS',4,'exposuretime','1"',56,372);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (224,'CanonEOS',4,'exposuretime','0"8',59,373);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (225,'CanonEOS',4,'exposuretime','0"6',61,374);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (226,'CanonEOS',4,'exposuretime','0"5',64,375);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (227,'CanonEOS',4,'exposuretime','0"4',67,376);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (228,'CanonEOS',4,'exposuretime','0"3(1/3)',69,377);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (229,'CanonEOS',4,'exposuretime','1/4',72,378);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (230,'CanonEOS',4,'exposuretime','1/5',75,379);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (231,'CanonEOS',4,'exposuretime','1/6(1/3)',77,380);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (232,'CanonEOS',4,'exposuretime','1/8',80,381);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (233,'CanonEOS',4,'exposuretime','1/10(1/3)',83,382);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (234,'CanonEOS',4,'exposuretime','1/13',85,383);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (235,'CanonEOS',4,'exposuretime','1/15',88,384);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (236,'CanonEOS',4,'exposuretime','1/20(1/3)',91,385);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (237,'CanonEOS',4,'exposuretime','1/25',93,386);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (238,'CanonEOS',4,'exposuretime','1/30',96,387);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (239,'CanonEOS',4,'exposuretime','1/40',99,388);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (240,'CanonEOS',4,'exposuretime','1/50',101,389);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (241,'CanonEOS',4,'exposuretime','1/60',104,390);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (242,'CanonEOS',4,'exposuretime','1/80',107,391);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (243,'CanonEOS',4,'exposuretime','1/100',109,392);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (244,'CanonEOS',4,'exposuretime','1/125',112,393);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (245,'CanonEOS',4,'exposuretime','1/160',115,394);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (246,'CanonEOS',4,'exposuretime','1/200',117,395);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (247,'CanonEOS',4,'exposuretime','1/250',120,396);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (248,'CanonEOS',4,'exposuretime','1/320',123,397);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (249,'CanonEOS',4,'exposuretime','1/400',125,398);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (250,'CanonEOS',4,'exposuretime','1/500',128,399);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (251,'CanonEOS',4,'exposuretime','1/640',131,400);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (252,'CanonEOS',4,'exposuretime','1/800',133,401);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (253,'CanonEOS',4,'exposuretime','1/1000',136,402);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (254,'CanonEOS',4,'exposuretime','1/1250',139,403);
insert into "T_CameraConfig"(IX,"Camera","Series","ConfigType","ConfigText","ConfigValue","OrderID") values (255,'CanonEOS',4,'exposuretime','1/1600',141,404);

insert into "T_CaptureSetting"(IX,"Series","ImageSize","ImageQuality",RGB_ISO,"RGB_ExposureTime","RGB_Aperture",RGB_WB,UV_ISO,"UV_ExposureTime","UV_Aperture",UV_WB,PL_ISO,"PL_ExposureTime","PL_Aperture",PL_WB,NPL_ISO,"NPL_ExposureTime","NPL_Aperture",NPL_WB,"Gender") values (1,4,0,3,85,93,36,1,93,117,37,2,85,93,36,1,88,112,53,2,1);

INSERT INTO T_FacePhoto_DrawInfo_Template (Photo_DirType,Info,EditTime) VALUES ('_L','{"width": 768, "height": 1024, "version": "1", "points": [{"x": 328.0, "y": 445.0}, {"x": 288.0, "y": 415.0}, {"x": 265.0, "y": 369.0}, {"x": 192.0, "y": 354.0}, {"x": 179.0, "y": 527.0}, {"x": 191.0, "y": 614.0}, {"x": 277.0, "y": 711.0}, {"x": 389.0, "y": 691.0}, {"x": 515.0, "y": 557.0}, {"x": 613.0, "y": 557.0}, {"x": 562.0, "y": 420.0}, {"x": 464.0, "y": 463.0}, {"x": 402.0, "y": 459.0}], "color": "#ff0000", "type":"smooth_curve", "weight": 3}',CURRENT_TIMESTAMP);
INSERT INTO T_FacePhoto_DrawInfo_Template (Photo_DirType,Info,EditTime) VALUES ('_R','{"width": 768, "height": 1024, "version": "1", "points": [{"x": 280.0, "y": 497.0}, {"x": 218.0, "y": 479.0}, {"x": 156.0, "y": 439.0}, {"x": 117.0, "y": 536.0}, {"x": 208.0, "y": 575.0}, {"x": 298.0, "y": 692.0}, {"x": 380.0, "y": 770.0}, {"x": 519.0, "y": 747.0}, {"x": 593.0, "y": 643.0}, {"x": 537.0, "y": 427.0}, {"x": 529.0, "y": 350.0}, {"x": 452.0, "y": 375.0}, {"x": 453.0, "y": 420.0}], "color": "#ff0000", "type":"smooth_curve", "weight": 3}',CURRENT_TIMESTAMP);
