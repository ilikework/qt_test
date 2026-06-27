#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
typedef unsigned char byte;
#define WM_USER_ANALYSEPROCESS		WM_USER+1
//#define WM_ENDMOISTURETEST			WM_USER+2
//#define WM_UPDATEMOISTUREVALUE		WM_USER+3
//#define WM_ENDCURRENTMOISTURETEST	WM_USER+4

typedef struct 
{
//short bm;
long fsize;
long zero;
long offbits;
long bisize;
long width;
long height;
short planes;
short bitcount;
long compression;
long sizeimage;
long xp;
long yp;
long clrused;
long clrimportant;

//char rgb[256][4];
}bmp24_header;
 
typedef struct        
{             
//short bm;
bmp24_header _24_in_8;

char rgb[256][4];
}bmp8_header;

void bmp24_header_init(bmp24_header *bh,int width,int height,int bitcount);
void bmp8_header_init(bmp8_header *bh,int width,int height,int bitcount);

int get_mem2D(byte ***array2D, int rows, int columns);
void free_mem2Dbyte(byte **array2D);

typedef struct 
{
	int x_loc;
	int y_loc;
	bool removed;
}pel_find;

typedef struct 
{
	int x_loc;
	int y_loc;
}pels;

typedef struct 
{
	bool right_pel;
	long n_pel_find;
}pel_log;

typedef struct 
{
	pels edg_pel[500];
	pels others[1000];
	long total_pel;
	long other_pel;
}edg;

#define rgb2y(R,G,B) (array_r[R])+(array_g[G])+(array_b[B]+ 0.5)

double array_r[256]={0.00000000000000000,
		0.29899999999999999,
		0.59799999999999998,
		0.89700000000000002,
		1.1960000000000000,
		1.4949999999999999,
		1.7940000000000000,
		2.0930000000000000,
		2.3919999999999999,
		2.6909999999999998,
		2.9899999999999998,
		3.2889999999999997,
		3.5880000000000001,
		3.8870000000000000,
		4.1859999999999999,
		4.4849999999999994,
		4.7839999999999998,
		5.0830000000000002,
		5.3819999999999997,
		5.6810000000000000,
		5.9799999999999995,
		6.2789999999999999,
		6.5779999999999994,
		6.8769999999999998,
		7.1760000000000002,
		7.4749999999999996,
		7.7740000000000000,
		8.0730000000000004,
		8.3719999999999999,
		8.6709999999999994,
		8.9699999999999989,
		9.2690000000000001,
		9.5679999999999996,
		9.8669999999999991,
		10.166000000000000,
		10.465000000000000,
		10.763999999999999,
		11.062999999999999,
		11.362000000000000,
		11.661000000000000,
		11.959999999999999,
		12.259000000000000,
		12.558000000000000,
		12.856999999999999,
		13.155999999999999,
		13.455000000000000,
		13.754000000000000,
		14.052999999999999,
		14.352000000000000,
		14.651000000000000,
		14.949999999999999,
		15.248999999999999,
		15.548000000000000,
		15.847000000000000,
		16.146000000000001,
		16.445000000000000,
		16.744000000000000,
		17.042999999999999,
		17.341999999999999,
		17.640999999999998,
		17.939999999999998,
		18.239000000000001,
		18.538000000000000,
		18.837000000000000,
		19.135999999999999,
		19.434999999999999,
		19.733999999999998,
		20.032999999999998,
		20.332000000000001,
		20.631000000000000,
		20.930000000000000,
		21.228999999999999,
		21.527999999999999,
		21.826999999999998,
		22.125999999999998,
		22.425000000000001,
		22.724000000000000,
		23.023000000000000,
		23.321999999999999,
		23.620999999999999,
		23.919999999999998,
		24.218999999999998,
		24.518000000000001,
		24.817000000000000,
		25.116000000000000,
		25.414999999999999,
		25.713999999999999,
		26.012999999999998,
		26.311999999999998,
		26.611000000000001,
		26.910000000000000,
		27.209000000000000,
		27.507999999999999,
		27.806999999999999,
		28.105999999999998,
		28.404999999999998,
		28.704000000000001,
		29.003000000000000,
		29.302000000000000,
		29.600999999999999,
		29.899999999999999,
		30.198999999999998,
		30.497999999999998,
		30.796999999999997,
		31.096000000000000,
		31.395000000000000,
		31.693999999999999,
		31.992999999999999,
		32.292000000000002,
		32.591000000000001,
		32.890000000000001,
		33.189000000000000,
		33.488000000000000,
		33.786999999999999,
		34.085999999999999,
		34.384999999999998,
		34.683999999999997,
		34.982999999999997,
		35.281999999999996,
		35.580999999999996,
		35.879999999999995,
		36.179000000000002,
		36.478000000000002,
		36.777000000000001,
		37.076000000000001,
		37.375000000000000,
		37.673999999999999,
		37.972999999999999,
		38.271999999999998,
		38.570999999999998,
		38.869999999999997,
		39.168999999999997,
		39.467999999999996,
		39.766999999999996,
		40.065999999999995,
		40.364999999999995,
		40.664000000000001,
		40.963000000000001,
		41.262000000000000,
		41.561000000000000,
		41.859999999999999,
		42.158999999999999,
		42.457999999999998,
		42.756999999999998,
		43.055999999999997,
		43.354999999999997,
		43.653999999999996,
		43.952999999999996,
		44.251999999999995,
		44.550999999999995,
		44.850000000000001,
		45.149000000000001,
		45.448000000000000,
		45.747000000000000,
		46.045999999999999,
		46.344999999999999,
		46.643999999999998,
		46.942999999999998,
		47.241999999999997,
		47.540999999999997,
		47.839999999999996,
		48.138999999999996,
		48.437999999999995,
		48.736999999999995,
		49.036000000000001,
		49.335000000000001,
		49.634000000000000,
		49.933000000000000,
		50.231999999999999,
		50.530999999999999,
		50.829999999999998,
		51.128999999999998,
		51.427999999999997,
		51.726999999999997,
		52.025999999999996,
		52.324999999999996,
		52.623999999999995,
		52.922999999999995,
		53.222000000000001,
		53.521000000000001,
		53.820000000000000,
		54.119000000000000,
		54.417999999999999,
		54.716999999999999,
		55.015999999999998,
		55.314999999999998,
		55.613999999999997,
		55.912999999999997,
		56.211999999999996,
		56.510999999999996,
		56.809999999999995,
		57.108999999999995,
		57.408000000000001,
		57.707000000000001,
		58.006000000000000,
		58.305000000000000,
		58.603999999999999,
		58.902999999999999,
		59.201999999999998,
		59.500999999999998,
		59.799999999999997,
		60.098999999999997,
		60.397999999999996,
		60.696999999999996,
		60.995999999999995,
		61.294999999999995,
		61.593999999999994,
		61.893000000000001,
		62.192000000000000,
		62.491000000000000,
		62.789999999999999,
		63.088999999999999,
		63.387999999999998,
		63.686999999999998,
		63.985999999999997,
		64.284999999999997,
		64.584000000000003,
		64.882999999999996,
		65.182000000000002,
		65.480999999999995,
		65.780000000000001,
		66.078999999999994,
		66.378000000000000,
		66.676999999999992,
		66.975999999999999,
		67.274999999999991,
		67.573999999999998,
		67.872999999999990,
		68.171999999999997,
		68.471000000000004,
		68.769999999999996,
		69.069000000000003,
		69.367999999999995,
		69.667000000000002,
		69.965999999999994,
		70.265000000000001,
		70.563999999999993,
		70.863000000000000,
		71.161999999999992,
		71.460999999999999,
		71.759999999999991,
		72.058999999999997,
		72.358000000000004,
		72.656999999999996,
		72.956000000000003,
		73.254999999999995,
		73.554000000000002,
		73.852999999999994,
		74.152000000000001,
		74.450999999999993,
		74.750000000000000,
		75.048999999999992,
		75.347999999999999,
		75.646999999999991,
		75.945999999999998,
		76.244999999999990

};
double array_g[256]={0.00000000000000000,
		0.58699999999999997,
		1.1739999999999999,
		1.7609999999999999,
		2.3479999999999999,
		2.9349999999999996,
		3.5219999999999998,
		4.1090000000000000,
		4.6959999999999997,
		5.2829999999999995,
		5.8699999999999992,
		6.4569999999999999,
		7.0439999999999996,
		7.6309999999999993,
		8.2180000000000000,
		8.8049999999999997,
		9.3919999999999995,
		9.9789999999999992,
		10.565999999999999,
		11.152999999999999,
		11.739999999999998,
		12.327000000000000,
		12.914000000000000,
		13.500999999999999,
		14.087999999999999,
		14.674999999999999,
		15.261999999999999,
		15.848999999999998,
		16.436000000000000,
		17.023000000000000,
		17.609999999999999,
		18.196999999999999,
		18.783999999999999,
		19.370999999999999,
		19.957999999999998,
		20.544999999999998,
		21.131999999999998,
		21.718999999999998,
		22.305999999999997,
		22.892999999999997,
		23.479999999999997,
		24.067000000000000,
		24.654000000000000,
		25.241000000000000,
		25.827999999999999,
		26.414999999999999,
		27.001999999999999,
		27.588999999999999,
		28.175999999999998,
		28.762999999999998,
		29.349999999999998,
		29.936999999999998,
		30.523999999999997,
		31.110999999999997,
		31.697999999999997,
		32.284999999999997,
		32.872000000000000,
		33.458999999999996,
		34.045999999999999,
		34.632999999999996,
		35.219999999999999,
		35.806999999999995,
		36.393999999999998,
		36.980999999999995,
		37.567999999999998,
		38.155000000000001,
		38.741999999999997,
		39.329000000000001,
		39.915999999999997,
		40.503000000000000,
		41.089999999999996,
		41.677000000000000,
		42.263999999999996,
		42.850999999999999,
		43.437999999999995,
		44.024999999999999,
		44.611999999999995,
		45.198999999999998,
		45.785999999999994,
		46.372999999999998,
		46.959999999999994,
		47.546999999999997,
		48.134000000000000,
		48.720999999999997,
		49.308000000000000,
		49.894999999999996,
		50.481999999999999,
		51.068999999999996,
		51.655999999999999,
		52.242999999999995,
		52.829999999999998,
		53.416999999999994,
		54.003999999999998,
		54.590999999999994,
		55.177999999999997,
		55.764999999999993,
		56.351999999999997,
		56.939000000000000,
		57.525999999999996,
		58.113000000000000,
		58.699999999999996,
		59.286999999999999,
		59.873999999999995,
		60.460999999999999,
		61.047999999999995,
		61.634999999999998,
		62.221999999999994,
		62.808999999999997,
		63.395999999999994,
		63.982999999999997,
		64.569999999999993,
		65.156999999999996,
		65.744000000000000,
		66.331000000000003,
		66.917999999999992,
		67.504999999999995,
		68.091999999999999,
		68.679000000000002,
		69.265999999999991,
		69.852999999999994,
		70.439999999999998,
		71.027000000000001,
		71.613999999999990,
		72.200999999999993,
		72.787999999999997,
		73.375000000000000,
		73.961999999999989,
		74.548999999999992,
		75.135999999999996,
		75.722999999999999,
		76.310000000000002,
		76.896999999999991,
		77.483999999999995,
		78.070999999999998,
		78.658000000000001,
		79.244999999999990,
		79.831999999999994,
		80.418999999999997,
		81.006000000000000,
		81.592999999999989,
		82.179999999999993,
		82.766999999999996,
		83.353999999999999,
		83.940999999999988,
		84.527999999999992,
		85.114999999999995,
		85.701999999999998,
		86.289000000000001,
		86.875999999999991,
		87.462999999999994,
		88.049999999999997,
		88.637000000000000,
		89.223999999999990,
		89.810999999999993,
		90.397999999999996,
		90.984999999999999,
		91.571999999999989,
		92.158999999999992,
		92.745999999999995,
		93.332999999999998,
		93.919999999999987,
		94.506999999999991,
		95.093999999999994,
		95.680999999999997,
		96.268000000000001,
		96.854999999999990,
		97.441999999999993,
		98.028999999999996,
		98.616000000000000,
		99.202999999999989,
		99.789999999999992,
		100.37700000000000,
		100.96400000000000,
		101.55099999999999,
		102.13799999999999,
		102.72499999999999,
		103.31200000000000,
		103.89900000000000,
		104.48599999999999,
		105.07299999999999,
		105.66000000000000,
		106.24700000000000,
		106.83399999999999,
		107.42099999999999,
		108.00800000000000,
		108.59500000000000,
		109.18199999999999,
		109.76899999999999,
		110.35599999999999,
		110.94300000000000,
		111.52999999999999,
		112.11699999999999,
		112.70399999999999,
		113.29100000000000,
		113.87800000000000,
		114.46499999999999,
		115.05199999999999,
		115.63900000000000,
		116.22600000000000,
		116.81299999999999,
		117.39999999999999,
		117.98699999999999,
		118.57400000000000,
		119.16099999999999,
		119.74799999999999,
		120.33499999999999,
		120.92200000000000,
		121.50899999999999,
		122.09599999999999,
		122.68299999999999,
		123.27000000000000,
		123.85700000000000,
		124.44399999999999,
		125.03099999999999,
		125.61799999999999,
		126.20500000000000,
		126.79199999999999,
		127.37899999999999,
		127.96599999999999,
		128.55300000000000,
		129.13999999999999,
		129.72700000000000,
		130.31399999999999,
		130.90099999999998,
		131.48800000000000,
		132.07499999999999,
		132.66200000000001,
		133.24900000000000,
		133.83599999999998,
		134.42300000000000,
		135.00999999999999,
		135.59699999999998,
		136.18400000000000,
		136.77099999999999,
		137.35800000000000,
		137.94499999999999,
		138.53199999999998,
		139.11900000000000,
		139.70599999999999,
		140.29299999999998,
		140.88000000000000,
		141.46699999999998,
		142.05400000000000,
		142.64099999999999,
		143.22799999999998,
		143.81500000000000,
		144.40199999999999,
		144.98900000000000,
		145.57599999999999,
		146.16299999999998,
		146.75000000000000,
		147.33699999999999,
		147.92399999999998,
		148.51100000000000,
		149.09799999999998,
		149.68500000000000

};
double array_b[256]={0.00000000000000000,
		0.11400000000000000,
		0.22800000000000001,
		0.34200000000000003,
		0.45600000000000002,
		0.57000000000000006,
		0.68400000000000005,
		0.79800000000000004,
		0.91200000000000003,
		1.0260000000000000,
		1.1400000000000001,
		1.2540000000000000,
		1.3680000000000001,
		1.4820000000000000,
		1.5960000000000001,
		1.7100000000000000,
		1.8240000000000001,
		1.9380000000000002,
		2.0520000000000000,
		2.1659999999999999,
		2.2800000000000002,
		2.3940000000000001,
		2.5080000000000000,
		2.6219999999999999,
		2.7360000000000002,
		2.8500000000000001,
		2.9640000000000000,
		3.0780000000000003,
		3.1920000000000002,
		3.3060000000000000,
		3.4199999999999999,
		3.5340000000000003,
		3.6480000000000001,
		3.7620000000000000,
		3.8760000000000003,
		3.9900000000000002,
		4.1040000000000001,
		4.2180000000000000,
		4.3319999999999999,
		4.4459999999999997,
		4.5600000000000005,
		4.6740000000000004,
		4.7880000000000003,
		4.9020000000000001,
		5.0160000000000000,
		5.1299999999999999,
		5.2439999999999998,
		5.3580000000000005,
		5.4720000000000004,
		5.5860000000000003,
		5.7000000000000002,
		5.8140000000000001,
		5.9279999999999999,
		6.0419999999999998,
		6.1560000000000006,
		6.2700000000000005,
		6.3840000000000003,
		6.4980000000000002,
		6.6120000000000001,
		6.7260000000000000,
		6.8399999999999999,
		6.9540000000000006,
		7.0680000000000005,
		7.1820000000000004,
		7.2960000000000003,
		7.4100000000000001,
		7.5240000000000000,
		7.6379999999999999,
		7.7520000000000007,
		7.8660000000000005,
		7.9800000000000004,
		8.0939999999999994,
		8.2080000000000002,
		8.3220000000000010,
		8.4359999999999999,
		8.5500000000000007,
		8.6639999999999997,
		8.7780000000000005,
		8.8919999999999995,
		9.0060000000000002,
		9.1200000000000010,
		9.2340000000000000,
		9.3480000000000008,
		9.4619999999999997,
		9.5760000000000005,
		9.6899999999999995,
		9.8040000000000003,
		9.9180000000000010,
		10.032000000000000,
		10.146000000000001,
		10.260000000000000,
		10.374000000000001,
		10.488000000000000,
		10.602000000000000,
		10.716000000000001,
		10.830000000000000,
		10.944000000000001,
		11.058000000000000,
		11.172000000000001,
		11.286000000000000,
		11.400000000000000,
		11.514000000000001,
		11.628000000000000,
		11.742000000000001,
		11.856000000000000,
		11.970000000000001,
		12.084000000000000,
		12.198000000000000,
		12.312000000000001,
		12.426000000000000,
		12.540000000000001,
		12.654000000000000,
		12.768000000000001,
		12.882000000000000,
		12.996000000000000,
		13.110000000000001,
		13.224000000000000,
		13.338000000000001,
		13.452000000000000,
		13.566000000000001,
		13.680000000000000,
		13.794000000000000,
		13.908000000000001,
		14.022000000000000,
		14.136000000000001,
		14.250000000000000,
		14.364000000000001,
		14.478000000000000,
		14.592000000000001,
		14.706000000000001,
		14.820000000000000,
		14.934000000000001,
		15.048000000000000,
		15.162000000000001,
		15.276000000000000,
		15.390000000000001,
		15.504000000000001,
		15.618000000000000,
		15.732000000000001,
		15.846000000000000,
		15.960000000000001,
		16.074000000000002,
		16.187999999999999,
		16.302000000000000,
		16.416000000000000,
		16.530000000000001,
		16.644000000000002,
		16.757999999999999,
		16.872000000000000,
		16.986000000000001,
		17.100000000000001,
		17.214000000000002,
		17.327999999999999,
		17.442000000000000,
		17.556000000000001,
		17.670000000000002,
		17.783999999999999,
		17.898000000000000,
		18.012000000000000,
		18.126000000000001,
		18.240000000000002,
		18.353999999999999,
		18.468000000000000,
		18.582000000000001,
		18.696000000000002,
		18.810000000000002,
		18.923999999999999,
		19.038000000000000,
		19.152000000000001,
		19.266000000000002,
		19.379999999999999,
		19.494000000000000,
		19.608000000000001,
		19.722000000000001,
		19.836000000000002,
		19.949999999999999,
		20.064000000000000,
		20.178000000000001,
		20.292000000000002,
		20.406000000000002,
		20.520000000000000,
		20.634000000000000,
		20.748000000000001,
		20.862000000000002,
		20.975999999999999,
		21.090000000000000,
		21.204000000000001,
		21.318000000000001,
		21.432000000000002,
		21.545999999999999,
		21.660000000000000,
		21.774000000000001,
		21.888000000000002,
		22.002000000000002,
		22.116000000000000,
		22.230000000000000,
		22.344000000000001,
		22.458000000000002,
		22.571999999999999,
		22.686000000000000,
		22.800000000000001,
		22.914000000000001,
		23.028000000000002,
		23.141999999999999,
		23.256000000000000,
		23.370000000000001,
		23.484000000000002,
		23.598000000000003,
		23.712000000000000,
		23.826000000000001,
		23.940000000000001,
		24.054000000000002,
		24.167999999999999,
		24.282000000000000,
		24.396000000000001,
		24.510000000000002,
		24.624000000000002,
		24.738000000000000,
		24.852000000000000,
		24.966000000000001,
		25.080000000000002,
		25.194000000000003,
		25.308000000000000,
		25.422000000000001,
		25.536000000000001,
		25.650000000000002,
		25.763999999999999,
		25.878000000000000,
		25.992000000000001,
		26.106000000000002,
		26.220000000000002,
		26.334000000000000,
		26.448000000000000,
		26.562000000000001,
		26.676000000000002,
		26.790000000000003,
		26.904000000000000,
		27.018000000000001,
		27.132000000000001,
		27.246000000000002,
		27.359999999999999,
		27.474000000000000,
		27.588000000000001,
		27.702000000000002,
		27.816000000000003,
		27.930000000000000,
		28.044000000000000,
		28.158000000000001,
		28.272000000000002,
		28.386000000000003,
		28.500000000000000,
		28.614000000000001,
		28.728000000000002,
		28.842000000000002,
		28.956000000000000,
		29.070000000000000

};


void init_face();
int draw_circle(byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,int total_find);
int find_edg(int origin_x,int origin_y,int y, int x, int self_direction);
int find_pn_edg(int origin_x,int origin_y,int y, int x, int self_direction);
bool check_with_direction(int y, int x, int direction);
bool check_pel(int y,int x);
bool check_edg(int y,int x);
int face_ana(int maxadd,int width,int height,byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,int r,int g, int b);
bool cal_edg(edg* edg_to_cal,int total_find,int max_val,int min_val);
bool cal_edg_winkel(edg* edg_to_cal,int total_find,int max_val,int min_val);
int cal_edg_one(edg* edg_to_cal);
int face_porphyrin_ana(int min_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd);
int draw_maokong(byte **colorbuffer_gray1,int total_find);
int draw_porphyrin(byte **colorbuffer_gray1,int total_find);
//int porphyrin_ana(int min_val,int *total_ports,byte **colorbuffer_gray1,byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,
//					  int width,int height,int r,int g,int b);
//int spots_ana(int *total_ports,byte **colorbuffer_gray1,byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,
//					  int width,int height);
int draw_spots(byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,int total_find);
int face_brownspots_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,byte **colorbuffer_gray2,int avg_all,int avg_b);
int face_maokong_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd);
int face_maokong_ana1(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd);
int face_spots_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd);
int face_spots_ana2(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all);
int face_spots_ana3(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all);
int face_spots_pn_ana2(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all);
int face_spots_pn_ana3(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all);
int face_spots_pn_ana4(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all);
int face_spots_pn_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd);
int draw_one (byte **colorbuffer_gray1,int y, int x, int total_find);
int face_one_ana(int gray,int width,int height,byte **colorbuffer_gray1);
int one_port_ana(byte **colorbuffer_gray1,int width,int height,int x,int y);

void in_out(int *x,int *y,int n,int width,int height,byte **in_0);	
int porphyrin_ana_new(byte **pGray,int nWidth,int nHeight,int *pnX,int *pnY,
					  INT_PTR nPointCount,int *pnPortCount,int *pnArea,int nMin,HWND hWnd);
int maokong_ana_new(byte **pGray,int nWidth,int nHeight,int *pnX,int *pnY,
					  INT_PTR nPointCount,int *pnPortCount,int *pnArea,int nMin
					  ,int nMax,int con_val,HWND hWnd,int version);
int draw_sports_new(byte **colorbuffer_gray1,int total_find,int max_area,int min_area);
int draw_winkel_new(byte **colorbuffer_gray1,int total_find,int max_area,int min_area);
int draw_pn_sports_new(byte **colorbuffer_gray1,int total_find,int max_area,int min_area);
int brownsports_ana_new(byte **pGray,byte **bGray,int nWidth,int nHeight,int *spX,int *spY,
						int nPointCount,int *spPortCount,int *spArea,
						int nMin,int nMax,int max_area,int min_area,int con_val,HWND hWnd);
int sports_ana_new(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
					  INT_PTR nPointCount,int *spPortCount,int *spArea,int nMin,
					  int nMax,int max_area,int min_area,int con_val,HWND hWnd,int version);
int sports_pn_new(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
					  int nPointCount,int *nPortCount, int *spArea,int nMin,int nMax,
					  int max_area,int min_area,int con_val,HWND hWnd,int version);
int winkel_ana_new(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
					  INT_PTR nPointCount,int *spArea,int nMin,int nMax,
					  int max_area,int min_area,int con_val,HWND hWnd);
int winkel_ana_new1(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
					  INT_PTR nPointCount,int *spArea,int nMin,int nMax,
					  int max_area,int min_area,int con_val,HWND hWnd);
int face_winkel_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd);
int face_winkel_ana1(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd);
int winkel_line(byte **pGray,byte **in_01,int width,int height);
int evenness_ana(byte **colorbuffer_gray,int *spX,int *spY,
				   INT_PTR nPointCount,int width,int height,float *result);
int contrast(byte** colorbuffer_gray,int width,int height,int val);
void Differential(byte** image_in,byte** image_out,int xsize,int ysize,float amp);
int winkel_kuo(byte **pGray,byte **in_01,byte **orig,int width,int height,int yu);
bool thinning(byte** pGray,int width,int height);
bool erasepixel(byte** pGray,int width,int height,int n);

pel_log buf_pel_log[3264][2448];
pel_find buf_pel_find[3264*2448/4];
edg buf_edg[8000];
int total_edg;
void bmp24_header_init(bmp24_header *bh,int width,int height,int bitcount)
{
	int i;
	bh->fsize=width*height*bitcount/8+sizeof(*bh)+2+(bitcount<24?1024:0)+((4-(width*bitcount/8)%4)%4)*height;
	bh->zero=0;
	bh->offbits=sizeof(*bh)+2+(bitcount<24?1024:0);
	bh->bisize=40;
	bh->width=width;
	bh->height=height;
	bh->planes=1;
	bh->bitcount=bitcount;
	bh->compression=0;
	bh->sizeimage=width*height*bitcount/8;
	bh->xp=3780;
	bh->yp=3780;
	for(i=0,bh->clrused=1;i<bitcount;i++) bh->clrused*=2;
	bh->clrimportant=0;

}

void bmp8_header_init(bmp8_header *bh,int width,int height,int bitcount)
{
	bmp24_header_init(&bh->_24_in_8,width,height,bitcount);
	int i;
	for (i = 0 ; i < 256;i++)
	{
		bh->rgb[i][0] = i;
		bh->rgb[i][1] = i;
		bh->rgb[i][2] = i;
		bh->rgb[i][3] = 0;
	}
		
}



int get_mem2D(byte ***array2D, int rows, int columns)
{
  int i;

  if((*array2D      = (byte**)calloc(rows,        sizeof(byte*))) == NULL)
    printf("get_mem2D: array2D");
  
  if(((*array2D)[0] = (byte* )calloc(columns*rows,sizeof(byte))) == NULL)
    printf("get_mem2D: array2D");

  for(i=1;i<rows;i++)
    (*array2D)[i] = (*array2D)[i-1] + columns ;

  return rows*columns;
}

void free_mem2Dbyte(byte **array2D)
{
  if (array2D)
  {
    if (array2D[0]) 
      free (array2D[0]);
    
    free (array2D);
  }

}

void init_face()
{
	int l1;
	int l2;
	int l3;
	total_edg = 0;
	for (l1 = 0 ; l1 < 8000 ; l1++)
	{
		buf_edg[l1].total_pel = 0; 
		buf_edg[l1].other_pel = 0;
		for (l2 = 0; l2 < 500; l2++)
		{
			buf_edg[l1].edg_pel[l2].x_loc = 0;
			buf_edg[l1].edg_pel[l2].y_loc = 0;
		}
		for (l3 = 0; l3 < 1000; l3++)
		{
			buf_edg[l1].others[l3].x_loc = 0;
			buf_edg[l1].others[l3].y_loc = 0;
		}
	}
	for (l1 = 0 ; l1 < 3264*2448/4 ; l1++)
	{
		buf_pel_find[l1].removed = false;
		buf_pel_find[l1].x_loc = 0;
		buf_pel_find[l1].y_loc = 0;
	}
	for (l1 = 0; l1 < 3264; l1++)
		for (l2 = 0; l2 < 2448; l2++)
		{
			buf_pel_log[l1][l2].n_pel_find = 0;
			buf_pel_log[l1][l2].right_pel = false;
		}
}


//int porphyrin_ana(int min_val,int *total_ports,byte **colorbuffer_gray1,byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,
//					  int width,int height,int r,int g,int b)
//{
//	int pels_find =0;
////	pels_find = face_porphyrin_ana(min_val,width,height,colorbuffer_gray1);
//	
////	draw_porphyrin(colorbuffer_r1,colorbuffer_g1,colorbuffer_b1,pels_find);
//	*total_ports = total_edg;
//	return pels_find;
//}

//int spots_ana(int *total_spots,byte **colorbuffer_gray1,byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,
//					  int width,int height)
//{
//	int pels_find =0;
////	pels_find = face_spots_ana(100,140,width,height,colorbuffer_gray1);
//	draw_spots(colorbuffer_r1,colorbuffer_g1,colorbuffer_b1,pels_find);
//	*total_spots = total_edg;
//	return pels_find;
//}

int draw_spots(byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,int total_find)
{
	bool find = false;
	int draw_total_pels = total_find;
	int l = 0;
	int top_y = 0;
	int top_x = 0;
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	while (!find)
	{
	
		for ( l = 0 ; l < total_find ; l++)
		{
			if (!buf_pel_find[l].removed)
			{
				//buf_pel_find[l].removed = true;
				check_x = buf_pel_find[l].x_loc;
				check_y = buf_pel_find[l].y_loc;
				break;
			}
		}

		if (l >= total_find - 1)
			find = true;
		while (check_pel(++check_y,check_x));

		origin_x = check_x;
		origin_y = --check_y;

		printf("origin_x: %d origin_y: %d\n",origin_x,origin_y);
		
		int find1 = find_edg(origin_x,origin_y,check_y,check_x,7);
		printf("find: %d\n",find1);

		if (!cal_edg(&buf_edg[total_edg],total_find,20,0)&&find == 1)
		{
			int i = 0;
			for ( i= 0 ; i < buf_edg[total_edg].other_pel;i++)
			{
				int c_y = buf_edg[total_edg].others[i].y_loc;
				int c_x = buf_edg[total_edg].others[i].x_loc;
				colorbuffer_b1[c_y][c_x] = 0;
				colorbuffer_r1[c_y][c_x] = 255;
				colorbuffer_g1[c_y][c_x] = 255;
			}
			for (i = 0 ; i < buf_edg[total_edg].total_pel;i++)
			{
				int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
				int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
				colorbuffer_b1[c_y][c_x] = 255;
				colorbuffer_r1[c_y][c_x] = 0;
				colorbuffer_g1[c_y][c_x] = 0;
			}
		}
		total_edg++;
		draw_total_pels -= buf_edg[total_edg].total_pel;
//		if (draw_total_pels <= 5)
//			find = true;
	}
	
	return 0;

}

#define BAND 12
int face_maokong_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd=NULL)
{
	/*int blocksize = 100;
	int pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	int i,j,m,n;
	int avg;
	int pel_count;
	int step = 50/h_loop;
	int step1 = 20+step;
	//first
	for (i = 0;i < h_loop;i++)
	{
	for (j = 0;j < w_loop;j++)
	{
	avg = 0;
	int minus = 0;
	pel_count = 0;
	for(m = 0;m < blocksize;m++)
	{
	for(n = 0;n < blocksize;n++)
	{

	if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]==0)
	minus++;
	else
	avg +=colorbuffer_gray1[blocksize*i+m][blocksize*j+n];
	}
	}
	if (minus == blocksize*blocksize)
	avg /= blocksize*blocksize;
	else
	avg /= (blocksize*blocksize-minus);
	if (avg>160||avg<50)
	continue;
	max_val = avg * 0.9;
	min_val = max_val - 20;
	for(m = 0;m < blocksize;m++)
	{
	for(n = 0;n < blocksize;n++)
	{
	if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]>=min_val&&colorbuffer_gray1[blocksize*i+m][blocksize*j+n] <= max_val)
	{
	buf_pel_find[pel].x_loc = blocksize*j+n;
	buf_pel_find[pel].y_loc = blocksize*i+m;
	buf_pel_find[pel].removed = false;
	buf_pel_log[blocksize*i+m][blocksize*j+n].n_pel_find = pel;
	buf_pel_log[blocksize*i+m][blocksize*j+n].right_pel = true;
	pel++;
	}
	else
	{
	buf_pel_log[blocksize*i+m][blocksize*j+n].right_pel = false;
	buf_pel_log[blocksize*i+m][blocksize*j+n].n_pel_find = 0;
	}
	}
	}
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,step1,0);
	step1+=step;
	}
	}

	/*for ( j = 0 ; j < height ; j++)
	for ( i = 0 ; i < width ; i++)
	{
	if (colorbuffer_gray1[j][i]>=min_val&&colorbuffer_gray1[j][i] <= max_val)
	{
	buf_pel_find[pel].x_loc = i;
	buf_pel_find[pel].y_loc = j;
	buf_pel_find[pel].removed = false;
	buf_pel_log[j][i].n_pel_find = pel;
	buf_pel_log[j][i].right_pel = true;
	pel++;
	}
	else
	{
	buf_pel_log[j][i].right_pel = false;
	buf_pel_log[j][i].n_pel_find = 0;
	}
	}
	return pel;*/
	int pel = 0;
	int current_val = 0;
	int step1 = 70;
#if 1
	//bool v_check,h_check,c1_check/* "\" */,c2_check;/* "/" */
	int u_counter,d_counter,l_counter,r_counter,ur_counter,rd_counter,dl_counter,lu_counter;
	int k;
	for (int j = BAND ; j < height-BAND ; j++)
		for (int i = BAND ; i < width-BAND ; i++)
		{
			current_val = colorbuffer_gray1[j][i];
			if (colorbuffer_gray1[j][i] > avg_all*1.7 || current_val == 0 || current_val == 255)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}
			// "|"
			u_counter = d_counter = 0;
			for ( k = 6;k < 12;k++)
			{
				if ((colorbuffer_gray1[j - k][i] - current_val) > 10)
					u_counter++;
				if ((colorbuffer_gray1[j + k][i] - current_val) > 10)
					d_counter++;
			}
			if (u_counter < 3 || d_counter < 3)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			// "-"
			l_counter = r_counter = 0;
			for ( k = 6;k < 12;k++)
			{
				if ((colorbuffer_gray1[j ][i - k] - current_val) > 10)
					l_counter++;
				if ((colorbuffer_gray1[j ][i + k] - current_val) > 10)
					r_counter++;
			}
			if (l_counter < 3 || r_counter < 3)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			// "\"
			lu_counter = rd_counter = 0;
			for ( k = 6;k < 12;k++)
			{
				if ((colorbuffer_gray1[j - k][i - k] - current_val) > 10)
					lu_counter++;
				if ((colorbuffer_gray1[j + k][i + k] - current_val) > 10)
					rd_counter++;
			}
			if (lu_counter < 2 || rd_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			// "/"
			ur_counter = dl_counter = 0;
			for ( k = 6;k < 12;k++)
			{
				if ((colorbuffer_gray1[j - k][i - k] - current_val) > 10)
					ur_counter++;
				if ((colorbuffer_gray1[j + k][i + k] - current_val) > 10)
					dl_counter++;
			}
			if (ur_counter < 2 || dl_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			buf_pel_find[pel].x_loc = i;
			buf_pel_find[pel].y_loc = j;
			buf_pel_find[pel].removed = false;
			buf_pel_log[j][i].n_pel_find = pel;
			buf_pel_log[j][i].right_pel = true;
			pel++;


		}
		LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,step1,0);
#else
	for (int j = 0 ; j < height ; j++)
		for (int i = 0 ; i < width ; i++)
		{
			if (colorbuffer_gray1[j][i]>=min_val&&colorbuffer_gray1[j][i] < 255)
			{
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				buf_pel_find[pel].removed = false;
				buf_pel_log[j][i].n_pel_find = pel;
				buf_pel_log[j][i].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
#endif
		return pel;
}
#undef BAND

#define BAND 14
int face_maokong_ana1(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd=NULL)
{
	
	int pel = 0;
	int current_val = 0;
	int step1 = 70;
#if 1
	//bool v_check,h_check,c1_check/* "\" */,c2_check;/* "/" */
	int u_counter,d_counter,l_counter,r_counter,ur_counter,rd_counter,dl_counter,lu_counter;
	int k;
	for (int j = BAND ; j < height-BAND ; j++)
		for (int i = BAND ; i < width-BAND ; i++)
		{
			current_val = colorbuffer_gray1[j][i];
			if (colorbuffer_gray1[j][i] > avg_all*2 || current_val == 0 || current_val == 255)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}
			// "|"
			u_counter = d_counter = 0;
			for ( k = 8;k < 14;k++)
			{
				if ((colorbuffer_gray1[j - k][i] - current_val) > 8)
					u_counter++;
				if ((colorbuffer_gray1[j + k][i] - current_val) > 8)
					d_counter++;
			}
			if (u_counter < 2 || d_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			// "-"
			l_counter = r_counter = 0;
			for ( k = 8;k < 14;k++)
			{
				if ((colorbuffer_gray1[j ][i - k] - current_val) > 8)
					l_counter++;
				if ((colorbuffer_gray1[j ][i + k] - current_val) > 8)
					r_counter++;
			}
			if (l_counter < 2 || r_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}
			
			// "\"
			lu_counter = rd_counter = 0;
			for ( k = 8;k < 14;k++)
			{
				if ((colorbuffer_gray1[j - k][i - k] - current_val) > 8)
					lu_counter++;
				if ((colorbuffer_gray1[j + k][i + k] - current_val) > 8)
					rd_counter++;
			}
			if (lu_counter < 2 || rd_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			// "/"
			ur_counter = dl_counter = 0;
			for ( k = 8;k < 14;k++)
			{
				if ((colorbuffer_gray1[j - k][i - k] - current_val) > 8)
					ur_counter++;
				if ((colorbuffer_gray1[j + k][i + k] - current_val) > 8)
					dl_counter++;
			}
			if (ur_counter < 2 || dl_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}
			
			buf_pel_find[pel].x_loc = i;
			buf_pel_find[pel].y_loc = j;
			buf_pel_find[pel].removed = false;
			buf_pel_log[j][i].n_pel_find = pel;
			buf_pel_log[j][i].right_pel = true;
			pel++;
			

		}
		LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,step1,0);
#else
	for (int j = 0 ; j < height ; j++)
		for (int i = 0 ; i < width ; i++)
		{
			if (colorbuffer_gray1[j][i]>=min_val&&colorbuffer_gray1[j][i] < 255)
			{
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				buf_pel_find[pel].removed = false;
				buf_pel_log[j][i].n_pel_find = pel;
				buf_pel_log[j][i].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
#endif
	return pel;
}
#undef BAND

int face_brownspots_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,byte **colorbuffer_gray2,int avg_all,int avg_b)
{
	int blocksize = 25;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	float coff_high,coff_low,coff_test;
	coff_test = (float)min_val/10;
	coff_high = avg_all > 200 ? 1.1f:1.25f ;
	coff_low = avg_all > 200 ? 0.87f:coff_test;
	int i,j,m,n;
	int pel_count;
	byte **temp_buffer;
	get_mem2D(&temp_buffer,height,width);

	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (colorbuffer_gray1[i][j] < 20 || colorbuffer_gray1[i][j] > 200)
			{
				temp_buffer[i][j]=0;
				continue;
			}

			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = -blocksize;m < blocksize;m++)
			{
				for(n = -blocksize;n < blocksize;n++)
				{
					if (colorbuffer_gray1[i+m][j+n]==255 || colorbuffer_gray1[i+m][j+n]==0)
						minus++;
					else
						avg +=colorbuffer_gray1[i+m][j+n];
				}
			}
			if (minus == blocksize*blocksize*4)
				avg /= blocksize*blocksize*4;
			else
				avg /= (blocksize*blocksize*4-minus);
			//			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
			//				continue;

			max_val = avg > 190 ? (int)(avg*0.86):(int)(avg*0.93);

			min_val = max_val - 110;
			min_val = min_val<1?1:min_val;

			if (colorbuffer_gray1[i][j]>=min_val&&colorbuffer_gray1[i][j] <= max_val)
			{
				temp_buffer[i][j]=1;
			}
			else
			{
				temp_buffer[i][j]=0;
			}
		}
	}

	int loop = 2;
	while (loop--)
	{
		for (i = 10;i < height-10;i++)
		{
			for (j = 10;j < width-10;j++)
			{
				if (temp_buffer[i][j] == 1 )
				{
					for(m = -1;m < 1;m++)
						for(n = -1;n < 1;n++)
						{
							int minus;
							minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
							if (minus <= 5 && temp_buffer[i+m][j+n] == 0)
							{
								temp_buffer[i+m][j+n]=1;
							}
						}
				}
			}
		}
	}
	coff_test = (float)min_val/10;
	coff_high = avg_b > 200 ? 1.1f:1.25f;
	coff_low = avg_b > 200 ? 0.87f:coff_test;
	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (colorbuffer_gray2[i][j] < 20 || colorbuffer_gray2[i][j] > 200 || temp_buffer[i][j] == 1)
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
				continue;
			}

			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = -blocksize;m < blocksize;m++)
			{
				for(n = -blocksize;n < blocksize;n++)
				{
					if (colorbuffer_gray2[i+m][j+n]==255 || colorbuffer_gray2[i+m][j+n]==0)
						minus++;
					else
						avg +=colorbuffer_gray2[i+m][j+n];
				}
			}
			if (minus == blocksize*blocksize*4)
				avg /= blocksize*blocksize*4;
			else
				avg /= (blocksize*blocksize*4-minus);
			//			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
			//				continue;
			max_val = avg > 190 ? (int)(avg*0.86):(int)(avg*0.93);

			min_val = max_val - 110;
			min_val = min_val<1?1:min_val;

			if (colorbuffer_gray2[i][j]>=min_val&&colorbuffer_gray2[i][j] <= max_val)
			{
				buf_pel_find[pel].x_loc = j;
				buf_pel_find[pel].y_loc = i;
				buf_pel_find[pel].removed = false;
				buf_pel_log[i][j].n_pel_find = pel;
				buf_pel_log[i][j].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
			}
		}
	}

	loop = 2;
	while (loop--)
	{
		for (i = 10;i < height-10;i++)
		{
			for (j = 10;j < width-10;j++)
			{
				if (buf_pel_log[i][j].right_pel == true )
				{
					for(m = -1;m < 1;m++)
						for(n = -1;n < 1;n++)
						{
							int minus;
							minus = abs(colorbuffer_gray2[i+m][j+n] - colorbuffer_gray2[i][j]);
							if (minus <= 5 && buf_pel_log[i+m][j+n].right_pel == false)
							{
								buf_pel_find[pel].x_loc = j+n;
								buf_pel_find[pel].y_loc = i+m;
								buf_pel_find[pel].removed = false;
								buf_pel_log[i+m][j+n].n_pel_find = pel;
								buf_pel_log[i+m][j+n].right_pel = true;
								pel++;
							}
						}
				}
			}
		}
	}
	return pel;
	free_mem2Dbyte(temp_buffer);
}
int face_spots_ana2(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all)
{
	int blocksize = 25;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	float coff_high,coff_low,coff_test;
	coff_test = (float)min_val/10;
	coff_high = avg_all > 200 ? 1.1f:1.25f ;
	coff_low = avg_all > 200 ? 0.87f:coff_test;
	int i,j,m,n;
	int pel_count;

	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (colorbuffer_gray1[i][j] < 20 || colorbuffer_gray1[i][j] > 200)
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
				continue;
			}

			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = -blocksize;m < blocksize;m++)
			{
				for(n = -blocksize;n < blocksize;n++)
				{
					if (colorbuffer_gray1[i+m][j+n]==255 || colorbuffer_gray1[i+m][j+n]==0)
						minus++;
					else
						avg +=colorbuffer_gray1[i+m][j+n];
				}
			}
			if (minus == blocksize*blocksize*4)
				avg /= blocksize*blocksize*4;
			else
				avg /= (blocksize*blocksize*4-minus);
			//			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
			//				continue;
			max_val = avg > 190 ? (int)(avg*0.86):(int)(avg*0.93);

			min_val = max_val - 110;
			min_val = min_val<1?1:min_val;

			if (colorbuffer_gray1[i][j]>=min_val&&colorbuffer_gray1[i][j] <= max_val)
			{
				buf_pel_find[pel].x_loc = j;
				buf_pel_find[pel].y_loc = i;
				buf_pel_find[pel].removed = false;
				buf_pel_log[i][j].n_pel_find = pel;
				buf_pel_log[i][j].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
			}
		}
	}

	int loop = 2;
	while (loop--)
	{
		for (i = 10;i < height-10;i++)
		{
			for (j = 10;j < width-10;j++)
			{
				if (buf_pel_log[i][j].right_pel == true )
				{
					for(m = -1;m < 1;m++)
						for(n = -1;n < 1;n++)
						{
							int minus;
							minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
							if (minus <= 5 && buf_pel_log[i+m][j+n].right_pel == false)
							{
								buf_pel_find[pel].x_loc = j+n;
								buf_pel_find[pel].y_loc = i+m;
								buf_pel_find[pel].removed = false;
								buf_pel_log[i+m][j+n].n_pel_find = pel;
								buf_pel_log[i+m][j+n].right_pel = true;
								pel++;
							}
						}
				}
			}
		}
	}
	return pel;
}
int face_spots_ana3(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all)
{
	int blocksize = 25;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	float coff_high,coff_low,coff_test;
	coff_test = (float)min_val/10;
	coff_high = avg_all > 200 ? 1.1f:1.25f ;
	coff_low = avg_all > 200 ? 0.87f:coff_test;
	int i,j,m,n;
	int pel_count;

	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (colorbuffer_gray1[i][j] < 20 || colorbuffer_gray1[i][j] > 200)
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
				continue;
			}

			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = -blocksize;m < blocksize;m++)
			{
				for(n = -blocksize;n < blocksize;n++)
				{
					if (colorbuffer_gray1[i+m][j+n]==255 || colorbuffer_gray1[i+m][j+n]==0)
						minus++;
					else
						avg +=colorbuffer_gray1[i+m][j+n];
				}
			}
			if (minus == blocksize*blocksize*4)
				avg /= blocksize*blocksize*4;
			else
				avg /= (blocksize*blocksize*4-minus);
//			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
//				continue;
			max_val = avg > 190 ? (int)(avg*0.87):(int)(avg*0.96);
			
			min_val = max_val - 140;
			min_val = min_val<1?1:min_val;

			if (colorbuffer_gray1[i][j]>=min_val&&colorbuffer_gray1[i][j] <= max_val)
			{
				buf_pel_find[pel].x_loc = j;
				buf_pel_find[pel].y_loc = i;
				buf_pel_find[pel].removed = false;
				buf_pel_log[i][j].n_pel_find = pel;
				buf_pel_log[i][j].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
			}
		}
	}

	int loop = 2;
	while (loop--)
	{
		for (i = 10;i < height-10;i++)
		{
			for (j = 10;j < width-10;j++)
			{
				if (buf_pel_log[i][j].right_pel == true )
				{
					for(m = -1;m < 1;m++)
						for(n = -1;n < 1;n++)
						{
							int minus;
							minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
							if (minus <= 5 && buf_pel_log[i+m][j+n].right_pel == false)
							{
								buf_pel_find[pel].x_loc = j+n;
								buf_pel_find[pel].y_loc = i+m;
								buf_pel_find[pel].removed = false;
								buf_pel_log[i+m][j+n].n_pel_find = pel;
								buf_pel_log[i+m][j+n].right_pel = true;
								pel++;
							}
						}
				}
			}
		}
	}
	return pel;
}
int face_spots_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd=NULL)
{
	int blocksize = 100;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	float coff_high,coff_low,coff_test;
	coff_test = (float)min_val/10;
	coff_high = avg_all > 200 ? 1.02f:1.25f ;
	coff_low = avg_all > 200 ? 0.87f:coff_test;
	int i,j,m,n;
	int pel_count;
	//first
	for (i = 0;i < h_loop;i++)
	{
		for (j = 0;j < w_loop;j++)
		{
			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]==0)
						minus++;
					else
						avg +=colorbuffer_gray1[blocksize*i+m][blocksize*j+n];
				}
			}
			if (minus == blocksize*blocksize)
				avg /= blocksize*blocksize;
			else
				avg /= (blocksize*blocksize-minus);
			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
				continue;
			max_val = avg > 190? (int)(avg*0.70):(int)(avg*0.75);
			
			min_val = max_val - 110;
			min_val = min_val<1?1:min_val;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]>=min_val&&colorbuffer_gray1[blocksize*i+m][blocksize*j+n] <= max_val)
					{
						pel_count++;
					}
				}
			}
			if (pel_count > 5000||pel_count < 200)
				continue;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]>=min_val&&colorbuffer_gray1[blocksize*i+m][blocksize*j+n] <= max_val)
					{
						buf_pel_find[pel].x_loc = blocksize*j+n;
						buf_pel_find[pel].y_loc = blocksize*i+m;
						buf_pel_find[pel].removed = false;
						buf_pel_log[blocksize*i+m][blocksize*j+n].n_pel_find = pel;
						buf_pel_log[blocksize*i+m][blocksize*j+n].right_pel = true;
						pel++;
					}
					else
					{
						buf_pel_log[blocksize*i+m][blocksize*j+n].right_pel = false;
						buf_pel_log[blocksize*i+m][blocksize*j+n].n_pel_find = 0;
					}
				}
			}
		}
		
	}
	//second
	w_loop--;
	h_loop--;
	for (i = 0;i < h_loop;i++)
	{
		for (j = 0;j < w_loop;j++)
		{
			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20]==0)
						minus++;
					else
						avg +=colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20];
				}
			}
			if (minus == blocksize*blocksize)
				avg /= blocksize*blocksize;
			else
				avg /= (blocksize*blocksize-minus);
			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
				continue;
			max_val = avg > 190 ? (int)(avg*0.87):(int)(avg*0.95);
			
			min_val = max_val - 90;
			min_val = min_val<1?1:min_val;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20]>=min_val&&colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20] <= max_val)
					{
						pel_count++;
					}
				}
			}
			if (pel_count > 5000||pel_count < 200)
				continue;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20]>=min_val&&colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20] <= max_val&&buf_pel_log[blocksize*i+m+20][blocksize*j+n+20].right_pel == false)
					{
						buf_pel_find[pel].x_loc = blocksize*j+n+20;
						buf_pel_find[pel].y_loc = blocksize*i+m+20;
						buf_pel_find[pel].removed = false;
						buf_pel_log[blocksize*i+m+20][blocksize*j+n+20].n_pel_find = pel;
						buf_pel_log[blocksize*i+m+20][blocksize*j+n+20].right_pel = true;
						pel++;
					}
				}
			}
		}
		
	}
	// ???????
	int loop = 5;
	while (loop--)
	{
		for (i = 1;i < height-1;i++)
		{
			for (j = 1;j < width-1;j++)
			{
				if (buf_pel_log[i][j].right_pel == true )
				{
					for(m = -1;m < 1;m++)
						for(n = -1;n < 1;n++)
						{
							int minus;
							minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
							if (minus <= 7 && buf_pel_log[i+m][j+n].right_pel == false)
							{
								buf_pel_find[pel].x_loc = j+n;
								buf_pel_find[pel].y_loc = i+m;
								buf_pel_find[pel].removed = false;
								buf_pel_log[i+m][j+n].n_pel_find = pel;
								buf_pel_log[i+m][j+n].right_pel = true;
								pel++;
							}
						}
				}
			}
		}
	}
//	for (int j = 0 ; j < height ; j++)
//		for (int i = 0 ; i < width ; i++)
//		{
//			if (colorbuffer_gray1[j][i]>=min_val&&colorbuffer_gray1[j][i] <= max_val)
//			{
//				buf_pel_find[pel].x_loc = i;
//				buf_pel_find[pel].y_loc = j;
//				buf_pel_find[pel].removed = false;
//				buf_pel_log[j][i].n_pel_find = pel;
//				buf_pel_log[j][i].right_pel = true;
//				pel++;
//			}
//			else
//			{
//				buf_pel_log[j][i].right_pel = false;
//				buf_pel_log[j][i].n_pel_find = 0;
//			}
//		}
	return pel;
}
int face_spots_pn_ana2(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all)
{
	int blocksize = 25;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	int i,j,m,n;
	int pel_count;
	double tmp_avg;
	int tmp_min = min_val;
//	int min_val_c;
//	int max_val_c;
	double coff_max = (double)max_val/100;

	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (colorbuffer_gray1[i][j] < 20 || colorbuffer_gray1[i][j] > 200)
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
				continue;
			}

			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = -blocksize;m < blocksize;m++)
			{
				for(n = -blocksize;n < blocksize;n++)
				{
					if (colorbuffer_gray1[i+m][j+n]==0 || colorbuffer_gray1[i+m][j+n]==255)
						minus++;
					else
						avg +=colorbuffer_gray1[i+m][j+n];
				}
			}
			if (minus == blocksize*blocksize*4)
				avg /= blocksize*blocksize*4;
			else
				avg /= (blocksize*blocksize*4-minus);

			tmp_avg = avg - tmp_min;

			tmp_avg = tmp_avg < 0 ? 0 : tmp_avg;
			tmp_avg = tmp_avg > 60 ? 60 :tmp_avg;

			tmp_avg *= 20;
			tmp_avg /= 60;
			tmp_avg = 100 - tmp_avg;
			tmp_avg /= 100;

			tmp_avg = tmp_avg > 0.90 ? 0.90:tmp_avg;
			tmp_avg = tmp_avg < 0.70 ? 0.70:tmp_avg;

			max_val = avg > 200 ? (int)(avg*0.75):(int)(avg*coff_max);
			max_val = avg < 100 ? (int)(avg*0.85):(int)(avg*coff_max);
//			max_val = avg * tmp_avg;
			
			min_val = max_val - 110;
			min_val = min_val<1?1:min_val;

			if (colorbuffer_gray1[i][j]>=min_val&&colorbuffer_gray1[i][j] <= max_val)
			{
				buf_pel_find[pel].x_loc = j;
				buf_pel_find[pel].y_loc = i;
				buf_pel_find[pel].removed = false;
				buf_pel_log[i][j].n_pel_find = pel;
				buf_pel_log[i][j].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
			}
		}
	}

	int loop = 0;
	while (loop--)
	{
		for (i = 10;i < height-10;i++)
		{
			for (j = 10;j < width-10;j++)
			{
				if (buf_pel_log[i][j].right_pel == true )
				{
					for(m = -1;m < 1;m++)
						for(n = -1;n < 1;n++)
						{
							int minus;
							minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
							if (minus <= 5 && buf_pel_log[i+m][j+n].right_pel == false)
							{
								buf_pel_find[pel].x_loc = j+n;
								buf_pel_find[pel].y_loc = i+m;
								buf_pel_find[pel].removed = false;
								buf_pel_log[i+m][j+n].n_pel_find = pel;
								buf_pel_log[i+m][j+n].right_pel = true;
								pel++;
							}
						}
				}
			}
		}
	}
	return pel;
}
int face_spots_pn_ana3(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all)
{
	int blocksize = 25;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	int i,j,m,n;
	int pel_count;
	//double tmp_avg;
	int tmp_min = min_val;
	int *tmp_buffer;
	int *tmp_buffer1;
	//	int min_val_c;
	//	int max_val_c;
	double coff_max = (double)max_val/100;
	tmp_buffer =(int *)malloc(height*width*sizeof(int));
	tmp_buffer1 =(int *)malloc(height*width*sizeof(int));
	memset(tmp_buffer,0,height*width*sizeof(int));
	memset(tmp_buffer1,0,height*width*sizeof(int));

	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (colorbuffer_gray1[i][j] < 20 || colorbuffer_gray1[i][j] > 200)
			{
				tmp_buffer[i*width + j] = 0;
				continue;
			}

			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = -blocksize;m < blocksize;m++)
			{
				for(n = -blocksize;n < blocksize;n++)
				{
					if (colorbuffer_gray1[i+m][j+n]==0 || colorbuffer_gray1[i+m][j+n]==255)
						minus++;
					else
						avg +=colorbuffer_gray1[i+m][j+n];
				}
			}
			if (minus == blocksize*blocksize*4)
				avg /= blocksize*blocksize*4;
			else
				avg /= (blocksize*blocksize*4-minus);

			max_val = avg > 200 ? (int)(avg*0.80):(int)(avg*coff_max);
			max_val = avg < 100 ? (int)(avg*0.80):(int)(avg*coff_max);
			//			max_val = avg * tmp_avg;

			min_val = max_val - 110;
			min_val = min_val<1?1:min_val;

			if (colorbuffer_gray1[i][j]>=min_val&&colorbuffer_gray1[i][j] <= max_val)
			{
				tmp_buffer[i*width + j] = 1;
			}
		}
	}

	for (i = blocksize;i < height - blocksize ;i++)
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (tmp_buffer[i*width + j] == 1)
			{
				for(m = -7;m < 8;m++)
					for(n = -7;n < 8;n++)
					{
						int minus;
						minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
						if (minus <= 7 )
						{
							tmp_buffer1[(i+m)*width + (j+n)] = 1;
						}
					}
			}
		}

		//mid filter
		for (i = blocksize;i < height - blocksize ;i++)
			for (j = blocksize;j < width - blocksize;j++)
			{
				int counter = 0;
				for(m = -2;m < 3;m++)
					for(n = -2;n < 3;n++)
					{
						if(tmp_buffer1[(i+m)*width + (j+n)])
							counter++;
					}
					if (counter < 7)
						tmp_buffer[i*width + j] = 0;
					else
						tmp_buffer[i*width + j] = 1;
			}

			for (i = blocksize;i < height - blocksize ;i++)
			{
				for (j = blocksize;j < width - blocksize;j++)
				{
					if (tmp_buffer[i*width + j] == 1)
					{
						buf_pel_find[pel].x_loc = j;
						buf_pel_find[pel].y_loc = i;
						buf_pel_find[pel].removed = false;
						buf_pel_log[i][j].n_pel_find = pel;
						buf_pel_log[i][j].right_pel = true;
						pel++;
					}
					else
					{
						buf_pel_log[i][j].right_pel = false;
						buf_pel_log[i][j].n_pel_find = 0;
					}
				}
			}
			free(tmp_buffer);
			free(tmp_buffer1);
			return pel;
}
int face_spots_pn_ana4(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all)
{
	int blocksize = 25;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	int i,j,m,n;
	int pel_count;
	//double tmp_avg;
	int tmp_min = min_val;
	int *tmp_buffer;
	int *tmp_buffer1;
//	int min_val_c;
//	int max_val_c;
	double coff_max = (double)max_val/100;
	tmp_buffer =(int *)malloc(height*width*sizeof(int));
	tmp_buffer1 =(int *)malloc(height*width*sizeof(int));
	memset(tmp_buffer,0,height*width*sizeof(int));
	memset(tmp_buffer1,0,height*width*sizeof(int));

	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (colorbuffer_gray1[i][j] < 20 || colorbuffer_gray1[i][j] > 200)
			{
				tmp_buffer[i*width + j] = 0;
				continue;
			}

			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = -blocksize;m < blocksize;m++)
			{
				for(n = -blocksize;n < blocksize;n++)
				{
					if (colorbuffer_gray1[i+m][j+n]==0 || colorbuffer_gray1[i+m][j+n]==255)
						minus++;
					else
						avg +=colorbuffer_gray1[i+m][j+n];
				}
			}
			if (minus == blocksize*blocksize*4)
				avg /= blocksize*blocksize*4;
			else
				avg /= (blocksize*blocksize*4-minus);

			//avg > 80?(max_val = avg*0.90):(max_val = avg*coff_max);
			//avg < 60?(max_val = avg*0.90):(max_val = avg*coff_max);
			if (avg > 115)
			{
				max_val = (int)(avg*0.90);
			}
			else if (avg < 60)
			{
				max_val = (int)(avg*0.80);
			}
			else
			{
				max_val = (int)(avg*coff_max);
			}
//			max_val = avg * tmp_avg;
			
			min_val = max_val - 110;
			min_val = min_val<1?1:min_val;

			if (colorbuffer_gray1[i][j]>=min_val&&colorbuffer_gray1[i][j] <= max_val)
			{
				tmp_buffer[i*width + j] = 1;
			}
		}
	}

	for (i = blocksize;i < height - blocksize ;i++)
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (tmp_buffer[i*width + j] == 1)
			{
				for(m = -7;m < 8;m++)
					for(n = -7;n < 8;n++)
					{
						int minus;
						minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
						if (minus <= 7 )
						{
							tmp_buffer1[(i+m)*width + (j+n)] = 1;
						}
					}
			}
		}

	//mid filter
	for (i = blocksize;i < height - blocksize ;i++)
		for (j = blocksize;j < width - blocksize;j++)
		{
			int counter = 0;
			for(m = -2;m < 3;m++)
				for(n = -2;n < 3;n++)
				{
					if(tmp_buffer1[(i+m)*width + (j+n)])
						counter++;
				}
			if (counter < 7)
				tmp_buffer[i*width + j] = 0;
			else
				tmp_buffer[i*width + j] = 1;
		}

	for (i = blocksize;i < height - blocksize ;i++)
	{
		for (j = blocksize;j < width - blocksize;j++)
		{
			if (tmp_buffer[i*width + j] == 1)
			{
				buf_pel_find[pel].x_loc = j;
				buf_pel_find[pel].y_loc = i;
				buf_pel_find[pel].removed = false;
				buf_pel_log[i][j].n_pel_find = pel;
				buf_pel_log[i][j].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[i][j].right_pel = false;
				buf_pel_log[i][j].n_pel_find = 0;
			}
		}
	}
	free(tmp_buffer);
	free(tmp_buffer1);
	return pel;
}


int face_spots_pn_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd=NULL)
{
	int blocksize = 50;
	int pel = 0;
	int avg = 0;
	int sub_pel = 0;
	int w_loop = width/blocksize;
	int h_loop = height/blocksize;
	float coff_high,coff_low;
	coff_high = avg_all > 100 ? 1.9f:2.3f ;
	coff_low = avg_all > 100 ? 1.8f:1.3f;
	int i,j,m,n;
	int pel_count;
	//first
	for (i = 0;i < h_loop;i++)
	{
		for (j = 0;j < w_loop;j++)
		{
			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]==0)
						minus++;
					else
						avg +=colorbuffer_gray1[blocksize*i+m][blocksize*j+n];
				}
			}
			if (minus == blocksize*blocksize)
				avg /= blocksize*blocksize;
			else
				avg /= (blocksize*blocksize-minus);
//			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
//				continue;
			max_val = avg > 150 ? (int)(avg*0.69):(int)(avg*0.82);
			
			min_val = max_val - 120;
			min_val = min_val<1?1:min_val;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]>=min_val&&colorbuffer_gray1[blocksize*i+m][blocksize*j+n] <= max_val)
					{
						pel_count++;
					}
				}
			}
			if (pel_count > 7000)
				continue;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m][blocksize*j+n]>=min_val&&colorbuffer_gray1[blocksize*i+m][blocksize*j+n] <= max_val)
					{
						buf_pel_find[pel].x_loc = blocksize*j+n;
						buf_pel_find[pel].y_loc = blocksize*i+m;
						buf_pel_find[pel].removed = false;
						buf_pel_log[blocksize*i+m][blocksize*j+n].n_pel_find = pel;
						buf_pel_log[blocksize*i+m][blocksize*j+n].right_pel = true;
						pel++;
					}
					else
					{
						buf_pel_log[blocksize*i+m][blocksize*j+n].right_pel = false;
						buf_pel_log[blocksize*i+m][blocksize*j+n].n_pel_find = 0;
					}
				}
			}
		}
		
	}
	//  second
    w_loop--;
	h_loop--;
	for (i = 0;i < h_loop;i++)
	{
		for (j = 0;j < w_loop;j++)
		{
			avg = 0;
			int minus = 0;
			pel_count = 0;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20]==0)
						minus++;
					else
						avg +=colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20];
				}
			}
			if (minus == blocksize*blocksize)
				avg /= blocksize*blocksize;
			else
				avg /= (blocksize*blocksize-minus);
			if (avg >= avg_all*coff_high || avg <= avg_all*coff_low)
				continue;
			max_val = avg > 190 ? (int)(avg*0.69):(int)(avg*0.87);
			
			min_val = max_val - 90;
			min_val = min_val<10?10:min_val;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20]>=min_val&&colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20] <= max_val)
					{
						pel_count++;
					}
				}
			}
			if (pel_count > 5000)
				continue;
			for(m = 0;m < blocksize;m++)
			{
				for(n = 0;n < blocksize;n++)
				{
					if (colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20]>=min_val&&colorbuffer_gray1[blocksize*i+m+20][blocksize*j+n+20] <= max_val&&buf_pel_log[blocksize*i+m+20][blocksize*j+n+20].right_pel == false)
					{
						buf_pel_find[pel].x_loc = blocksize*j+n+20;
						buf_pel_find[pel].y_loc = blocksize*i+m+20;
						buf_pel_find[pel].removed = false;
						buf_pel_log[blocksize*i+m+20][blocksize*j+n+20].n_pel_find = pel;
						buf_pel_log[blocksize*i+m+20][blocksize*j+n+20].right_pel = true;
						pel++;
					}
				}
			}
		}
		
	}
	// ???????
	int loop = 5;
	while (loop--)
	{
		for (i = 1;i < height-1;i++)
		{
			for (j = 1;j < width-1;j++)
			{
				if (buf_pel_log[i][j].right_pel == true )
				{
					for(m = -1;m < 1;m++)
						for(n = -1;n < 1;n++)
						{
							int minus;
							minus = abs(colorbuffer_gray1[i+m][j+n] - colorbuffer_gray1[i][j]);
							if (minus <= 7 && buf_pel_log[i+m][j+n].right_pel == false)
							{
								buf_pel_find[pel].x_loc = j+n;
								buf_pel_find[pel].y_loc = i+m;
								buf_pel_find[pel].removed = false;
								buf_pel_log[i+m][j+n].n_pel_find = pel;
								buf_pel_log[i+m][j+n].right_pel = true;
								pel++;
							}
						}
				}
			}
		}
	}

//	for (int j = 0 ; j < height ; j++)
//		for (int i = 0 ; i < width ; i++)
//		{
//			if (colorbuffer_gray1[j][i]>=min_val&&colorbuffer_gray1[j][i] <= max_val)
//			{
//				buf_pel_find[pel].x_loc = i;
//				buf_pel_find[pel].y_loc = j;
//				buf_pel_find[pel].removed = false;
//				buf_pel_log[j][i].n_pel_find = pel;
//				buf_pel_log[j][i].right_pel = true;
//				pel++;
//			}
//			else
//			{
//				buf_pel_log[j][i].right_pel = false;
//				buf_pel_log[j][i].n_pel_find = 0;
//			}
//		}
	return pel;
}
#define BAND 6
int face_porphyrin_ana(int min_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd=NULL)
{
	int pel = 0;
	int step = 40/height;
	int step1 = 10+step;
#if 1
int u_counter,d_counter,l_counter,r_counter,ur_counter,rd_counter,dl_counter,lu_counter,current_val;
	int k;
	for (int j = BAND ; j < height-BAND ; j++)
	{
		for (int i = BAND ; i < width-BAND ; i++)
		{
			current_val = colorbuffer_gray1[j][i];
			if (colorbuffer_gray1[j][i] < avg_all*0.7 || current_val == 0 || current_val >= 200)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}
			// "|"
			u_counter = d_counter = 0;
			for ( k = 1;k < 6;k++)
			{
				if ((current_val - colorbuffer_gray1[j - k][i]) > 10)
					u_counter++;
				if ((current_val - colorbuffer_gray1[j + k][i]) > 10)
					d_counter++;
			}
			if (u_counter < 3 || d_counter < 3)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			// "-"
			l_counter = r_counter = 0;
			for ( k = 1;k < 6;k++)
			{
				if ((current_val - colorbuffer_gray1[j ][i - k]) > 10)
					l_counter++;
				if ((current_val - colorbuffer_gray1[j ][i + k]) > 10)
					r_counter++;
			}
			if (l_counter < 3 || r_counter < 3)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}
			
			// "\"
			lu_counter = rd_counter = 0;
			for ( k = 1;k < 4;k++)
			{
				if ((current_val - colorbuffer_gray1[j - k][i - k]) > 10)
					lu_counter++;
				if ((current_val - colorbuffer_gray1[j + k][i + k]) > 10)
					rd_counter++;
			}
			if (lu_counter < 2 || rd_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}

			// "/"
			ur_counter = dl_counter = 0;
			for ( k = 1;k < 4;k++)
			{
				if ((current_val - colorbuffer_gray1[j - k][i - k]) > 10)
					ur_counter++;
				if ((current_val - colorbuffer_gray1[j + k][i + k]) > 10)
					dl_counter++;
			}
			if (ur_counter < 2 || dl_counter < 2)
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
				continue;
			}
			
			buf_pel_find[pel].x_loc = i;
			buf_pel_find[pel].y_loc = j;
			buf_pel_find[pel].removed = false;
			buf_pel_log[j][i].n_pel_find = pel;
			buf_pel_log[j][i].right_pel = true;
			pel++;
			

		}
		LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,step1,0);
		step+=step;
	}
#else
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i++)
		{
			if (colorbuffer_gray1[j][i]>=min_val&&colorbuffer_gray1[j][i] < 255)
			{
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				buf_pel_find[pel].removed = false;
				buf_pel_log[j][i].n_pel_find = pel;
				buf_pel_log[j][i].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
		LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,step1,0);
		step+=step;
	}
#endif
	return pel;
}

int face_ana(int maxadd,int width,int height,byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,int r,int g, int b)
{
	int pel = 0;
	int c_r = 0;
	int c_g = 0;
	int c_b = 0;
	for (int j = 0 ; j < height ; j++)
		for (int i = 0 ; i < width ; i++)
		{
			c_r = abs(colorbuffer_r1[j][i]-r);
            c_g = abs(colorbuffer_g1[j][i]-g);
			c_b = abs(colorbuffer_b1[j][i]-b);
			if ((c_r+c_b+c_g)<=maxadd)
			{
				//colorbuffer_r1[j][i] = colorbuffer_g1[j][i] = colorbuffer_b1[j][i] = 0;
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				buf_pel_find[pel].removed = false;
				buf_pel_log[j][i].n_pel_find = pel;
				buf_pel_log[j][i].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
	return pel;
}

bool check_edg(int y,int x)
{
	if (x<0 || y<0)	return false; // add for maybe -1
	if (buf_pel_log[y][x].right_pel)
		if (!buf_pel_find[buf_pel_log[y][x].n_pel_find].removed)
		{
			buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
			buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
			//buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
			printf("find_x: %d find_y: %d\n",x,y);
			return true;		
		}
		else
			return false;
	else
		return false;
}

bool check_pel(int y,int x)
{
	if (buf_pel_log[y][x].right_pel)
		if (!buf_pel_find[buf_pel_log[y][x].n_pel_find].removed)
			return true;
		else
			return false;
	else
		return false;
}

int draw_circle(byte **colorbuffer_r1,byte **colorbuffer_g1,byte **colorbuffer_b1,int total_find)
{
	bool find = false;
	int draw_total_pels = total_find;
	int l = 0;
	int top_y = 0;
	int top_x = 0;
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	while (!find)
	{
	
		for ( l = 0 ; l < total_find ; l++)
		{
			if (!buf_pel_find[l].removed)
			{
				//buf_pel_find[l].removed = true;
				check_x = buf_pel_find[l].x_loc;
				check_y = buf_pel_find[l].y_loc;
				break;
			}
		}

		if (l >= total_find - 1)
			find = true;
		while (check_pel(++check_y,check_x));

		origin_x = check_x;
		origin_y = --check_y;

		printf("origin_x: %d origin_y: %d\n",origin_x,origin_y);
		
		int find1 = find_edg(origin_x,origin_y,check_y,check_x,7);
		printf("find: %d\n",find1);

		cal_edg(&buf_edg[total_edg],total_find,20,0);
		
			int i = 0;
			for (i = 0 ; i < buf_edg[total_edg].other_pel;i++)
			{
				int c_y = buf_edg[total_edg].others[i].y_loc;
				int c_x = buf_edg[total_edg].others[i].x_loc;
				colorbuffer_b1[c_y][c_x] = 0;
				colorbuffer_r1[c_y][c_x] = 0;
				colorbuffer_g1[c_y][c_x] = 0;
			}
			for (i = 0 ; i < buf_edg[total_edg].total_pel;i++)
			{
				int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
				int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
				colorbuffer_b1[c_y][c_x] = 0;
				colorbuffer_r1[c_y][c_x] = 255;
				colorbuffer_g1[c_y][c_x] = 255;
			}
		
		total_edg++;
		draw_total_pels -= buf_edg[total_edg].total_pel;
//		if (draw_total_pels <= 5)
//			find = true;
	}
	
	return 0;
}
int draw_maokong(byte **colorbuffer_gray1,int total_find)
{
	bool find = false;
	int draw_total_pels = total_find;
	int l = 0;
	int top_y = 0;
	int top_x = 0;
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	while (!find)
	{
	
		for ( l = 0 ; l < total_find ; l++)
		{
			if (!buf_pel_find[l].removed)
			{
				//buf_pel_find[l].removed = true;
				check_x = buf_pel_find[l].x_loc;
				check_y = buf_pel_find[l].y_loc;
				break;
			}
		}

		if ((l >= total_find - 1)||total_edg >= 7900)
			find = true;
		while (check_pel(++check_y,check_x));

		origin_x = check_x;
		origin_y = --check_y;

		printf("origin_x: %d origin_y: %d\n",origin_x,origin_y);
	
		int find1 = find_pn_edg(origin_x,origin_y,check_y,check_x,7);
		printf("find: %d\n",find1);

		if(cal_edg(&buf_edg[total_edg],total_find,30,1))
		{
			int i = 0 ;
			for (i =0; i < buf_edg[total_edg].other_pel;i++)
			{
				int c_y = buf_edg[total_edg].others[i].y_loc;
				int c_x = buf_edg[total_edg].others[i].x_loc;
				colorbuffer_gray1[c_y][c_x] = 255;
			}
			for (i = 0 ; i < buf_edg[total_edg].total_pel;i++)
			{
				int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
				int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
				colorbuffer_gray1[c_y][c_x] = colorbuffer_gray1[c_y-1][c_x] =
								colorbuffer_gray1[c_y+1][c_x] = colorbuffer_gray1[c_y][c_x-1] =
								colorbuffer_gray1[c_y][c_x-1] = colorbuffer_gray1[c_y][c_x-1] = 
								colorbuffer_gray1[c_y][c_x+1] = colorbuffer_gray1[c_y][c_x+1] = 
								colorbuffer_gray1[c_y][c_x+1] = 255;
			}
			total_edg++;
		}
		else
		{
			buf_edg[total_edg].total_pel = 0;
			buf_edg[total_edg].other_pel = 0;
		}
		draw_total_pels -= buf_edg[total_edg-1].total_pel;
//		if (draw_total_pels <= 5)
//			find = true;
	}
	
	return 0;
}
int draw_porphyrin(byte **colorbuffer_gray1,int total_find)
{
	bool find = false;
	int draw_total_pels = total_find;
	int l = 0;
	int top_y = 0;
	int top_x = 0;
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	while (!find)
	{
	
		for ( l = 0 ; l < total_find ; l++)
		{
			if (!buf_pel_find[l].removed)
			{
				//buf_pel_find[l].removed = true;
				check_x = buf_pel_find[l].x_loc;
				check_y = buf_pel_find[l].y_loc;
				break;
			}
		}
		
		if (l >= total_find - 1||total_edg >=8000-1)
			find = true;
		while (check_pel(++check_y,check_x));

		origin_x = check_x;
		origin_y = --check_y;

		printf("origin_x: %d origin_y: %d\n",origin_x,origin_y);
	
		int find1 = find_edg(origin_x,origin_y,check_y,check_x,7);
		printf("find: %d\n",find1);

		if(cal_edg(&buf_edg[total_edg],total_find,50,2))
		{
			int i = 0;
			for (i = 0 ; i < buf_edg[total_edg].other_pel;i++)
			{
				int c_y = buf_edg[total_edg].others[i].y_loc;
				int c_x = buf_edg[total_edg].others[i].x_loc;
				colorbuffer_gray1[c_y][c_x] = 255;
			}
			for (i = 0 ; i < buf_edg[total_edg].total_pel;i++)
			{
				int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
				int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
				colorbuffer_gray1[c_y][c_x] = 255;
#ifndef EXPAND
				colorbuffer_gray1[c_y-1][c_x] =
				colorbuffer_gray1[c_y+1][c_x] = colorbuffer_gray1[c_y][c_x-1] =
				colorbuffer_gray1[c_y][c_x-1] = colorbuffer_gray1[c_y][c_x-1] = 
				colorbuffer_gray1[c_y][c_x+1] = colorbuffer_gray1[c_y][c_x+1] = 
				colorbuffer_gray1[c_y][c_x+1] = 255;
#endif
			}
			total_edg++;
		}
		else
		{
			buf_edg[total_edg].total_pel = 0;
			buf_edg[total_edg].other_pel = 0;
		}
		draw_total_pels -= buf_edg[total_edg].total_pel;
//		if (draw_total_pels <= 5)
//			find = true;
	}
	
	return 0;
}
int cal_edg_one(edg* edg_to_cal)
{
	int max_y = 0;
	int max_x = 0;
	int min_y = 0;
	int min_x = 0;
	max_x=min_x=edg_to_cal->edg_pel[0].x_loc;
	max_y=min_y=edg_to_cal->edg_pel[0].y_loc;
	for (int i = 0;i < edg_to_cal->total_pel;i++)
	{
		edg_to_cal->edg_pel[i].x_loc > max_x ? max_x = edg_to_cal->edg_pel[i].x_loc :
		edg_to_cal->edg_pel[i].x_loc < min_x ? min_x = edg_to_cal->edg_pel[i].x_loc :min_x;
		edg_to_cal->edg_pel[i].y_loc > max_y ? max_y = edg_to_cal->edg_pel[i].y_loc :
		edg_to_cal->edg_pel[i].y_loc < min_y ? min_y = edg_to_cal->edg_pel[i].y_loc :max_y;	
	}
	int r1 = (max_x+max_y-min_y-min_x)/4;
	return (int)(r1*r1*3.2);
}
bool cal_edg_winkel(edg* edg_to_cal,int total_find,int max_val,int min_val)
{
	int i = 0;int j = 0;
	for (i = 0;i < edg_to_cal->total_pel;i++)
		for(j = 0;j < total_find;j++)
		{
			if (buf_pel_find[j].x_loc == edg_to_cal->edg_pel[i].x_loc&&
				buf_pel_find[j].y_loc == edg_to_cal->edg_pel[i].y_loc)
			{
				buf_pel_find[j].removed = true;
				continue;
			}
		}
		
	if (edg_to_cal->total_pel < max_val&&edg_to_cal->total_pel > min_val)
		return true;
	else
		return false;
}
bool cal_edg(edg* edg_to_cal,int total_find,int max_val,int min_val)
{
	//int min_val = 10;
	int temp_x1;
    int temp_y1;
	if (edg_to_cal->total_pel > 8)
	{
		temp_x1 =edg_to_cal->edg_pel[8].x_loc;
	    temp_y1 =edg_to_cal->edg_pel[8].y_loc;
	}
	else
	{
		temp_x1 = 0;
	    temp_y1 = 0;
	}
	bool not_cir = false;
	int max_y = 0;
	int max_x = 0;
	int min_y = 0;
	int min_x = 0;
	max_x=min_x=edg_to_cal->edg_pel[0].x_loc;
	max_y=min_y=edg_to_cal->edg_pel[0].y_loc;
	int i = 0;
	for ( i = 0;i < edg_to_cal->total_pel;i++)
	{
		if (i>8&&edg_to_cal->edg_pel[i].x_loc == temp_x1&&edg_to_cal->edg_pel[i].y_loc == temp_y1)
			not_cir = true;
		edg_to_cal->edg_pel[i].x_loc > max_x ? max_x = edg_to_cal->edg_pel[i].x_loc :
		edg_to_cal->edg_pel[i].x_loc < min_x ? min_x = edg_to_cal->edg_pel[i].x_loc :min_x;
		edg_to_cal->edg_pel[i].y_loc > max_y ? max_y = edg_to_cal->edg_pel[i].y_loc :
		edg_to_cal->edg_pel[i].y_loc < min_y ? min_y = edg_to_cal->edg_pel[i].y_loc :max_y;	
	}
//	printf("max_y: %d max_x: %d\n",max_y,max_x);
//	printf("min_y: %d min_x: %d\n",min_y,min_x);
	for (i = 0;i < total_find;i++)
	{
		if (buf_pel_find[i].x_loc >= min_x && buf_pel_find[i].x_loc <= max_x 
			&& buf_pel_find[i].y_loc >= min_y && buf_pel_find[i].y_loc <= max_y)
		{
			if (edg_to_cal->other_pel > 990)
				buf_pel_find[i].removed = true;
			else
			{
				buf_pel_find[i].removed = true;
				edg_to_cal->others[edg_to_cal->other_pel].x_loc = buf_pel_find[i].x_loc;
				edg_to_cal->others[edg_to_cal->other_pel].y_loc = buf_pel_find[i].y_loc;
				edg_to_cal->other_pel++;
			}
		}
		

	}
	if (edg_to_cal->total_pel < max_val&&edg_to_cal->total_pel > min_val)
	{	
		if (not_cir)
		{
			not_cir = false;
			return false;
		}
		else
			return true;
		
	}
	else
		return false;
	return true;
//	printf("total_pal: %d\n",edg_to_cal->total_pel);
}

bool check_with_direction(int y, int x,int direction)
{
	int check_y = y;
	int check_x = x;
	switch (direction)
	{
		case 0:
			check_x+=1;
			break;
		case 1:
			check_x+=1;
			check_y+=1;
			break;
		case 2:
			check_y+=1;
			break;
		case 3:
			check_x-=1;
			check_y+=1;
			break;
		case 4:
			check_x+=1;
			check_y-=1;
			break;
		case 5:
			check_y-=1;
			break;
		case 6:
			check_x-=1;
			check_y-=1;
			break;
		case 7:
			check_x-=1;
			break;
		default:
			return false;
 
	}

	if (check_edg(check_y,check_x))
		return true;
	else
		return false;
	
}
int find_pn_edg(int origin_x,int origin_y,int y, int x, int self_direction)
{
	if (buf_edg[total_edg].total_pel >= 1 && y == origin_y && x == origin_x)
		return 1;
	if ((x-origin_x)>=110||(y-origin_y)>=110)
		return 0;
	if (buf_edg[total_edg].total_pel>490)
		return 1;
	switch (self_direction)
	{
		case 0:
			if (check_with_direction(y,x,5))
				find_pn_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_pn_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_pn_edg(origin_x,origin_y,y,x-1,0); 
			else if (check_with_direction(y,x,3))
				find_pn_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_pn_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_pn_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_pn_edg(origin_x,origin_y,y,x+1,7);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 1:
			if (check_with_direction(y,x,4))
				find_pn_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_pn_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_pn_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_pn_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_pn_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_pn_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_pn_edg(origin_x,origin_y,y+1,x+1,6);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 2:
			if (check_with_direction(y,x,0))
				find_pn_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_pn_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_pn_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_pn_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_pn_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_pn_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_pn_edg(origin_x,origin_y,y+1,x,5);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 3:
			if (check_with_direction(y,x,1))
				find_pn_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_pn_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_pn_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_pn_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_pn_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_pn_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_pn_edg(origin_x,origin_y,y+1,x-1,4);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 4:
			if (check_with_direction(y,x,6))
				find_pn_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_pn_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_pn_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_pn_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_pn_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_pn_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_pn_edg(origin_x,origin_y,y-1,x+1,3);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 5:
			if (check_with_direction(y,x,7))
				find_pn_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_pn_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_pn_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_pn_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_pn_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_pn_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_pn_edg(origin_x,origin_y,y-1,x,2);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 6:
			if (check_with_direction(y,x,3))
				find_pn_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_pn_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_pn_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_pn_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_pn_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_pn_edg(origin_x,origin_y,y-1,x,1);
			else if (check_with_direction(y,x,6))
				find_pn_edg(origin_x,origin_y,y-1,x-1,1);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 7:
			if (check_with_direction(y,x,2))
				find_pn_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_pn_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_pn_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_pn_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_pn_edg(origin_x,origin_y,y-1,x,1);
			else if (check_with_direction(y,x,6))
				find_pn_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_pn_edg(origin_x,origin_y,y,x-1,0);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		default:
			break;
	}
	return 0;
}
int find_edg(int origin_x,int origin_y,int y, int x, int self_direction)
{
	//buf_pel_find[buf_pel_log[origin_y][origin_x].n_pel_find].removed = false;
	//printf("true: %d\n",buf_pel_find[buf_pel_log[origin_y][origin_x].n_pel_find].removed);
	if (buf_edg[total_edg].total_pel >= 1 && y == origin_y && x == origin_x)
		return 1;
	if ((y - origin_y) > 200||(x - origin_x) > 200)
		return 1;
	if (buf_edg[total_edg].total_pel>490)
		return 1;
	switch (self_direction)
	{
		case 0:
			if (check_with_direction(y,x,5))
				find_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_edg(origin_x,origin_y,y,x+1,7);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 1:
			if (check_with_direction(y,x,4))
				find_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_edg(origin_x,origin_y,y+1,x+1,6);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 2:
			if (check_with_direction(y,x,0))
				find_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_edg(origin_x,origin_y,y+1,x,5);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 3:
			if (check_with_direction(y,x,1))
				find_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_edg(origin_x,origin_y,y-1,x,2);
			else if (check_with_direction(y,x,6))
				find_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_edg(origin_x,origin_y,y+1,x-1,4);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 4:
			if (check_with_direction(y,x,6))
				find_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_edg(origin_x,origin_y,y-1,x+1,3);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 5:
			if (check_with_direction(y,x,7))
				find_edg(origin_x,origin_y,y,x-1,0);
			else if (check_with_direction(y,x,3))
				find_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_edg(origin_x,origin_y,y-1,x,2);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 6:
			if (check_with_direction(y,x,3))
				find_edg(origin_x,origin_y,y+1,x-1,4);
			else if (check_with_direction(y,x,2))
				find_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_edg(origin_x,origin_y,y-1,x,1);
			else if (check_with_direction(y,x,6))
				find_edg(origin_x,origin_y,y-1,x-1,1);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
		case 7:
			if (check_with_direction(y,x,2))
				find_edg(origin_x,origin_y,y+1,x,5);
			else if (check_with_direction(y,x,1))
				find_edg(origin_x,origin_y,y+1,x+1,6);
			else if (check_with_direction(y,x,0))
				find_edg(origin_x,origin_y,y,x+1,7);
			else if (check_with_direction(y,x,4))
				find_edg(origin_x,origin_y,y-1,x+1,3);
			else if (check_with_direction(y,x,5))
				find_edg(origin_x,origin_y,y-1,x,1);
			else if (check_with_direction(y,x,6))
				find_edg(origin_x,origin_y,y-1,x-1,1);
			else if (check_with_direction(y,x,7))
				find_edg(origin_x,origin_y,y,x-1,0);
			else
			{	
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel].x_loc = x;
				buf_edg[total_edg].edg_pel[buf_edg[total_edg].total_pel++].y_loc = y;
				buf_pel_find[buf_pel_log[y][x].n_pel_find].removed = true;
				return 0;
			}
			break;
	}
	return 0;
}

int one_port_ana(byte **colorbuffer_gray1,int width,int height,int x,int y)
{
	int gray_tmp1 = colorbuffer_gray1[y][x]
				   +colorbuffer_gray1[y][x-1]
				   +colorbuffer_gray1[y][x+1]
				   +colorbuffer_gray1[y-1][x]
				   +colorbuffer_gray1[y-1][x-1]
				   +colorbuffer_gray1[y-1][x+1]
				   +colorbuffer_gray1[y+1][x]
				   +colorbuffer_gray1[y+1][x-1]
				   +colorbuffer_gray1[y+1][x+1]; 
	gray_tmp1 /= 9;
	contrast(colorbuffer_gray1,width,height,20);
	int pels_find;
	pels_find = face_one_ana(gray_tmp1,width,height,colorbuffer_gray1);
	//buf_edg = (edg*)calloc(20,sizeof(edg));
	//printf("totalpel: %ld\n",pels_find);
	for (int j = 0;j < height;j++)
		for (int i = 0;i < width;i++)
		{
			colorbuffer_gray1[j][i] = 0;
		}
	int right1 = draw_one(colorbuffer_gray1,y,x,pels_find);
	int cha = width*height - pels_find;
	int area = cal_edg_one(&buf_edg[0]);
	if (cha <= width*height/4)
	{
		area = 0;
	}
	if (right1)
		return area;
	else
		return 0;

}

int face_one_ana(int gray,int width,int height,byte **colorbuffer_gray1)
{
	int pel = 0;int avg = 0;
//	for (int j = 0 ; j < height ; j++)
//		for (int i = 0 ; i < width ; i++)
//		{
//			avg+=colorbuffer_gray1[j][i];
//		}
//		avg /= height*width;
		int max_val;
		if (gray < 95)
			max_val = 30;
		else
			max_val = 15;
	for (int j = 0 ; j < height ; j++)
		for (int i = 0 ; i < width ; i++)
		{
			if (abs(gray - colorbuffer_gray1[j][i])<=max_val)
			{
				//colorbuffer_r1[j][i] = colorbuffer_g1[j][i] = colorbuffer_b1[j][i] = 0;
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				buf_pel_find[pel].removed = false;
				buf_pel_log[j][i].n_pel_find = pel;
				buf_pel_log[j][i].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
	return pel;
}

int draw_one (byte **colorbuffer_gray1,int y, int x, int total_find)
{
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	check_x = x;
	check_y = y;
	while (check_pel(++check_y,check_x));

	origin_x = check_x;
	origin_y = --check_y;

	int find1 = find_edg(origin_x,origin_y,check_y,check_x,7);
	if (find1)
	{
		for (int i = 0 ; i < buf_edg[total_edg].total_pel;i++)
		{
			int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
			int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
			colorbuffer_gray1[c_y][c_x] = 255;
		}
		return 1;
	}
	else
		return 0;
}

void in_out(int *x,int *y,int n,int width,int height,byte **in_0)
{
	int temp_x1=0;
	int temp_x2=0;
	int temp_y1=0;
	int temp_y2=0;
	float zb_k1=0;
	float zb_k2=0;
	float zb_siga=0;
    float zb_siga1=0;
	float zb_siga2=0;
	float zb_sigb=0;
	float zb_sigb1=0;
	float zb_sigb2=0;
	int max_x=0;
	int min_x=0;
	int max_y=0;
	int min_y=0;
	int temp_count1=0;
	int temp_count2=0;
	int temp_count3=0;
	int temp_count4=0;
	byte** temp_count;
    get_mem2D(&temp_count,height,width);
	int i;

		max_x=x[0];
		min_x=x[0];
		max_y=y[0];
		min_y=y[0];
		for(i=0;i<n;i++)
		{
			if(max_x<x[i]){max_x=*(x+i);}
			if(min_x>x[i]){min_x=*(x+i);}
			if(max_y<y[i]){max_y=*(y+i);}
			if(min_y>y[i]){min_y=*(y+i);}
		}
		for (i=min_y;i<=max_y;i++)
		{
			for (int j=min_x;j<=max_x;j++)
			{
				temp_count[i][j]=0;
			}
		}
		for (int j=0;j<width;j++)
		{
			for(i=0;i<height;i++)
			{
				in_0[i][j]=1;
			}
		}
		for (i=min_x;i<=max_x;i++)
		{
			for (int j=min_y;j<=max_y;j++)
			{
				temp_count1=0;
				temp_count2=0;
				temp_count3=0;
				temp_count4=0;
				int k=0;
				for(k = 0;k<n;k++)
				{
					temp_x1=*(x+k);
					temp_y1=*(y+k);
					//temp_x2=*(x+k+1); delete by lou 100212
					//temp_y2=*(y+k+1); delete by lou 100212
					if(k==n-1)           
					{
						temp_x2=*(x+0);
				 	    temp_y2=*(y+0);
					}
					else// add by lou 100212
					{
						temp_x2=*(x+k+1); // add by lou 100212
						temp_y2=*(y+k+1);// add by lou 100212
					}// add by lou 100212					
					if(temp_x2!=temp_x1) // add by lou 091002
						zb_k1=(float)(temp_y2-temp_y1)/(temp_x2-temp_x1);
					else
						zb_k1=(float)(temp_y2-temp_y1);
					zb_siga1=(float)(j-temp_y1-zb_k1*(i-temp_x1));
		            zb_siga2=(float)(j-temp_y1-zb_k1*(min_x-1-temp_x1));
		            zb_siga=zb_siga1*zb_siga2;
					zb_sigb1=(float)(temp_y1-j);
					zb_sigb2=(float)(temp_y2-j);
					zb_sigb=zb_sigb1*zb_sigb2;
					if(!((zb_siga>0)||(zb_sigb>0)))
					{temp_count1++;}
				}
				if(temp_count1%2==1){temp_count2++;}
				if(temp_count1==0){temp_count3++;}
				temp_count1=0;
				for(k=0;k<n;k++)
				{
					temp_x1=*(x+k);
					temp_y1=*(y+k);
					//temp_x2=*(x+k+1); delete by lou 100212
					//temp_y2=*(y+k+1); delete by lou 100212
					if(k==n-1)       
					{
						temp_x2=*(x+0);
				 	    temp_y2=*(y+0);
					}
					else // add by lou 100212
					{
						temp_x2=*(x+k+1); // add by lou 100212
						temp_y2=*(y+k+1); // add by lou 100212
					} // add by lou 100212
					if (temp_x2!=temp_x1) // add by lou 091002
						zb_k1=(float)(temp_y2-temp_y1)/(temp_x2-temp_x1);
					else
						zb_k1=(float)(temp_y2-temp_y1);
					zb_siga1=(float)(j-temp_y1-zb_k1*(i-temp_x1));
		            zb_siga2=(float)(j-temp_y1-zb_k1*(max_x+1-temp_x1));
		            zb_siga=zb_siga1*zb_siga2;
					zb_sigb1=(float)(temp_y1-j);
					zb_sigb2=(float)(temp_y2-j);
					zb_sigb=zb_sigb1*zb_sigb2;
					if(!((zb_siga>0)||(zb_sigb>0)))
					{temp_count1++;}
				}
				if(temp_count1%2==1){temp_count2++;}
				if(temp_count1==0){temp_count3++;}
				temp_count1=0;
				for(k=0;k<n;k++)
				{
					temp_x1=*(x+k);
					temp_y1=*(y+k);
					//temp_x2=*(x+k+1);delete by lou 100212
					//temp_y2=*(y+k+1);delete by lou 100212
					if(k==n-1)     
					{
						temp_x2=*(x+0);
				 	    temp_y2=*(y+0);
					}
					else // add by lou 100212
					{
						temp_x2=*(x+k+1); // add by lou 100212
						temp_y2=*(y+k+1); // add by lou 100212
					} // add by lou 100212
					if (temp_x2!=temp_x1) // add by lou 091002
						zb_k1=(float)(temp_y2-temp_y1)/(temp_x2-temp_x1);
					else
						zb_k1=(float)(temp_y2-temp_y1);
					zb_siga1=(float)(j-temp_y1-zb_k1*(i-temp_x1));
		            zb_siga2=(float)(min_y-1-temp_y1-zb_k1*(i-temp_x1));
		            zb_siga=zb_siga1*zb_siga2;
					zb_sigb1=(float)(temp_x1-i);
					zb_sigb2=(float)(temp_x2-i);
					zb_sigb=zb_sigb1*zb_sigb2;
					if(!((zb_siga>0)||(zb_sigb>0)))
					{temp_count1++;}
				}
				if(temp_count1%2==1){temp_count2++;}
				if(temp_count1==0){temp_count3++;}
				temp_count1=0;
				for(k=0;k<n;k++)
				{
					temp_x1=*(x+k);
					temp_y1=*(y+k);
					//temp_x2=*(x+k+1);delete by lou 100212
					//temp_y2=*(y+k+1);delete by lou 100212
					if(k==n-1)     
					{
						temp_x2=*(x+0);
				 	    temp_y2=*(y+0);
					}
					else// add by lou 100212
					{
						temp_x2=*(x+k+1);// add by lou 100212
						temp_y2=*(y+k+1);// add by lou 100212
					}// add by lou 100212
					if (temp_x2!=temp_x1) // add by lou 091002
						zb_k1=(float)(temp_y2-temp_y1)/(temp_x2-temp_x1);
					else
						zb_k1=(float)(temp_y2-temp_y1);
					zb_siga1=(float)(j-temp_y1-zb_k1*(i-temp_x1));
		            zb_siga2=(float)(max_y+1-temp_y1-zb_k1*(i-temp_x1));
		            zb_siga=zb_siga1*zb_siga2;
					zb_sigb1=(float)(temp_x1-i);
					zb_sigb2=(float)(temp_x2-i);
					zb_sigb=zb_sigb1*zb_sigb2;
					if(!((zb_siga>0)||(zb_sigb>0)))
					{temp_count1++;}
				}
				if(temp_count1%2==1){temp_count2++;}
				if(temp_count1==0){temp_count3++;}
				temp_count[j][i]=temp_count2++;
				if (j > 0 && i > 0 && temp_count[j-1][i-1]<2){temp_count4++;}
				if (j > 0 && temp_count[j-1][i]<2){temp_count4++;}
				if (i > 0 && temp_count[j][i-1]<2){temp_count4++;}
				if(temp_count3>0) 
					in_0[j][i]=1;
				else if(temp_count4>1)
					in_0[j][i]=1;
				else 
					in_0[j][i]=0;
			}
       }
	   free_mem2Dbyte(temp_count);
}
int maokong_ana_new(byte **pGray,int nWidth,int nHeight,int *pnX,int *pnY,
					  INT_PTR nPointCount,int *pnPortCount,int *pnArea,
					  int nMin,int nMax,int con_val,HWND hWnd=NULL,int version =1)
{
	byte **in_01;
	get_mem2D(&in_01,nHeight,nWidth);
	in_out(pnX,pnY,(int)nPointCount,nWidth,nHeight,in_01);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,10,0);
	int avg_tmp = 0;
	init_face();
	int pnArea_in = 1;
	int j = 0;
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (in_01[j][i]==1)
			{	
				pGray[j][i] = 0;
			}
			else
			{
				avg_tmp += pGray[j][i];
				pnArea_in++;
			}
		}
	}
	
	*pnArea = pnArea_in;
	avg_tmp /= pnArea_in;
	contrast(pGray,nWidth,nHeight,con_val);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,20,0);
	int pels_find;
	int tmpmax = nMax;
	nMax*=(100+con_val);
	nMax/=100;
	nMin +=(nMax - tmpmax);
	if(!version)
	  pels_find = face_maokong_ana(nMin,nMax,nWidth,nHeight,pGray,avg_tmp,hWnd);
	else
	  pels_find = face_maokong_ana1(nMin,nMax,nWidth,nHeight,pGray,avg_tmp,hWnd);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,70,0);
	//buf_edg = (edg*)calloc(10000,sizeof(edg));
	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,80,0);
//	for (int i = 0;i < pels_find;i++ )
//	{
//		int x1 = buf_pel_find[i].x_loc;
//		int y1 = buf_pel_find[i].y_loc;
//		pGray[y1][x1] = 255;
//	}
	draw_maokong(pGray,pels_find);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,100,0);
	*pnPortCount = total_edg;
	free_mem2Dbyte(in_01);
	return pels_find;
}
int porphyrin_ana_new(byte **pGray,int nWidth,int nHeight,int *pnX,int *pnY,
					  INT_PTR nPointCount,int *pnPortCount,int *pnArea,int nMin,HWND hWnd=NULL)
{
	byte **in_01;
	get_mem2D(&in_01,nHeight,nWidth);
	in_out(pnX,pnY,(int)nPointCount,nWidth,nHeight,in_01);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,10,0);
	int avg_tmp = 0;
	init_face();
	int pnArea_in = 1;
	int j = 0;
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (in_01[j][i]==1)
			{	
				pGray[j][i] = 0;
			}
			else
			{	
				avg_tmp+=pGray[j][i];
				pnArea_in++;
			}
		}
	}
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,20,0);
	*pnArea = pnArea_in;
	avg_tmp/=pnArea_in;
	int pels_find;
	pels_find = face_porphyrin_ana(nMin,nWidth,nHeight,pGray,avg_tmp,hWnd);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,60,0);
	//buf_edg = (edg*)calloc(10000,sizeof(edg));
	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;

	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,70,0);
	for (int i = 0;i < pels_find;i++ )
	{
		int x1 = buf_pel_find[i].x_loc;
		int y1 = buf_pel_find[i].y_loc;
		pGray[y1][x1] = 255;
	}
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,80,0);
	draw_porphyrin(pGray,pels_find);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,100,0);
	*pnPortCount = total_edg;
	free_mem2Dbyte(in_01);
	return pels_find;	
}

int sports_pn_new(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
					  int nPointCount,int *nPortCount, int *spArea,int nMin,int nMax,
					  int max_area,int min_area,int con_val,HWND hWnd=NULL,int version=1)
{
	byte **in_01;
	get_mem2D(&in_01,nHeight,nWidth);
	in_out(spX,spY,nPointCount,nWidth,nHeight,in_01);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,60,0);
	int avg_tmp = 0;
	init_face();
	int spArea_in = 1;
	int j = 0;
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (in_01[j][i]==1)
			{	
				pGray[j][i] = 0;
			}
			else
			{
				if (pGray[j][i] && pGray[j][i] < 255 )
				{
					avg_tmp+=pGray[j][i];
					spArea_in++;
				}
			}
		}
	}
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,70,0);
	*spArea = spArea_in;
	if (spArea_in>0)
		avg_tmp/=spArea_in;
	contrast(pGray,nWidth,nHeight,con_val);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,75,0);
	int pels_find;
/*	int tmpmax = nMax;
	nMax*=(100+con_val);
	nMax/=100;
	nMin +=(nMax - tmpmax);*/
	if(!version)
		pels_find = face_spots_pn_ana3(nMin,nMax,nWidth,nHeight,pGray,avg_tmp/*,hWnd*/);
	else
		pels_find = face_spots_pn_ana4(nMin,nMax,nWidth,nHeight,pGray,avg_tmp/*,hWnd*/);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,80,0);
	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,85,0);
/*	for (int i = 0;i < pels_find;i++ )
	{
		int x1 = buf_pel_find[i].x_loc;
		int y1 = buf_pel_find[i].y_loc;
		pGray[y1][x1] = 255;
	}*/

	
	draw_pn_sports_new(pGray,pels_find,max_area,min_area);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,100,0);
	*nPortCount = total_edg;
	free_mem2Dbyte(in_01);
	return pels_find;
}
int brownsports_ana_new(byte **pGray,byte **bGray,int nWidth,int nHeight,int *spX,int *spY,
				   int nPointCount,int *spPortCount,int *spArea,
				   int nMin,int nMax,int max_area,int min_area,int con_val,HWND hWnd=NULL)
{
	byte **in_01;
	get_mem2D(&in_01,nHeight,nWidth);
	in_out(spX,spY,nPointCount,nWidth,nHeight,in_01);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,10,0);
	int avg_tmp = 0;
	int avg_b = 0;
	init_face();
	int spArea_in = 1;
	int j = 0;
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (in_01[j][i]==1)
			{	
				pGray[j][i] = 0;
				bGray[j][i] = 0;
			}
			else
			{
				avg_tmp+=pGray[j][i];
				avg_b+=bGray[j][i];
				spArea_in++;
			}
		}
	}

	*spArea = spArea_in;
	avg_tmp/=spArea_in;
	contrast(pGray,nWidth,nHeight,con_val);
	contrast(bGray,nWidth,nHeight,con_val);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,15,0);
	int pels_find;
	int tmpmax = nMax;
	nMax*=(100+con_val);
	nMax/=100;
	nMin +=(nMax - tmpmax);
	pels_find = face_brownspots_ana(nMin,nMax,nWidth,nHeight,pGray,bGray,avg_tmp,avg_b);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,40,0);
	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;

	//	for (int i = 0;i < pels_find;i++ )
	//	{
	//		int x1 = buf_pel_find[i].x_loc;
	//		int y1 = buf_pel_find[i].y_loc;
	//		pGray[y1][x1] = 255;
	//	}


	draw_sports_new(bGray,pels_find,max_area,min_area);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,50,0);
	*spPortCount = total_edg;
	free_mem2Dbyte(in_01);
	return pels_find;
}
int sports_ana_new(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
				   INT_PTR nPointCount,int *spPortCount,int *spArea,
				   int nMin,int nMax,int max_area,int min_area,int con_val,HWND hWnd=NULL,int version=1)
{
	byte **in_01;
	get_mem2D(&in_01,nHeight,nWidth);
	in_out(spX,spY,(int)nPointCount,nWidth,nHeight,in_01);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,10,0);
	int avg_tmp = 0;
	init_face();
	int spArea_in = 1;
	int j = 0;
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (in_01[j][i]==1)
			{	
				pGray[j][i] = 0;
			}
			else
			{
				avg_tmp+=pGray[j][i];
				spArea_in++;
			}
		}
	}
	
	*spArea = spArea_in;
	avg_tmp/=spArea_in;
	contrast(pGray,nWidth,nHeight,con_val);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,15,0);
	int pels_find;
	int tmpmax = nMax;
	nMax*=(100+con_val);
	nMax/=100;
	nMin +=(nMax - tmpmax);
	if(!version)
		pels_find = face_spots_ana2(nMin,nMax,nWidth,nHeight,pGray,avg_tmp);//,hWnd);
	else
		pels_find = face_spots_ana3(nMin,nMax,nWidth,nHeight,pGray,avg_tmp);//,hWnd);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,40,0);
	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;

//	for (int i = 0;i < pels_find;i++ )
//	{
//		int x1 = buf_pel_find[i].x_loc;
//		int y1 = buf_pel_find[i].y_loc;
//		pGray[y1][x1] = 255;
//	}

	
	draw_sports_new(pGray,pels_find,max_area,min_area);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,50,0);
	*spPortCount = total_edg;
	free_mem2Dbyte(in_01);
	return pels_find;
}

int draw_pn_sports_new(byte **colorbuffer_gray1,int total_find,int max_area,int min_area)
{
	bool find = false;
	int draw_total_pels = total_find;
	int area = 0;
	int l = 0;
	int top_y = 0;
	int top_x = 0;
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	while (!find)
	{
	
		for ( l = 0 ; l < total_find ; l++)
		{
			if (!buf_pel_find[l].removed)
			{
				//buf_pel_find[l].removed = true;
				check_x = buf_pel_find[l].x_loc;
				check_y = buf_pel_find[l].y_loc;
				break;
			}
		}

		if (l >= total_find - 1)
			find = true;
		while (check_pel(++check_y,check_x));

		origin_x = check_x;
		origin_y = --check_y;

		printf("origin_x: %d origin_y: %d\n",origin_x,origin_y);
	
		int find1 = find_pn_edg(origin_x,origin_y,check_y,check_x,7);
		printf("find: %d\n",find1);

		if(cal_edg(&buf_edg[total_edg],total_find,max_area,min_area))
		{
			int i;
//			for (i = 0 ; i < buf_edg[total_edg].other_pel;i++)
//			{
//				int c_y = buf_edg[total_edg].others[i].y_loc;
//				int c_x = buf_edg[total_edg].others[i].x_loc;
//				//colorbuffer_gray1[c_y][c_x] = 255;
//			}
			for (i = 0 ; i < buf_edg[total_edg].total_pel;i++)
			{
				int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
				int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
				colorbuffer_gray1[c_y][c_x]  = 255;
			}
			area +=buf_edg[total_edg].total_pel;
			area +=buf_edg[total_edg].other_pel;
			total_edg++;
		}
		else
		{
			buf_edg[total_edg].total_pel = 0;
			buf_edg[total_edg].other_pel = 0;
			buf_pel_find[l].removed = true;
		}
		draw_total_pels -= buf_edg[total_edg].total_pel;
	}
	
	return area;
}
int draw_winkel_new(byte **colorbuffer_gray1,int total_find,int max_area,int min_area)
{	
	bool find = false;
	int draw_total_pels = total_find;
	int l = 0;
	int top_y = 0;
	int top_x = 0;
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	while (!find)
	{
	
		for ( l = 0 ; l < total_find ; l++)
		{
			if (!buf_pel_find[l].removed)
			{
				//buf_pel_find[l].removed = true;
				check_x = buf_pel_find[l].x_loc;
				check_y = buf_pel_find[l].y_loc;
				break;
			}
		}

		if (l >= total_find - 1)
			find = true;
		while (check_pel(++check_y,check_x));

		origin_x = check_x;
		origin_y = --check_y;

		printf("origin_x: %d origin_y: %d\n",origin_x,origin_y);
	
		int find1 = find_edg(origin_x,origin_y,check_y,check_x,7);
		printf("find: %d\n",find1);

		if(cal_edg_winkel(&buf_edg[total_edg],total_find,max_area,min_area))
		{
//			for (int i = 0 ; i < buf_edg[total_edg].other_pel;i++)
//			{
//				int c_y = buf_edg[total_edg].others[i].y_loc;
//				int c_x = buf_edg[total_edg].others[i].x_loc;
//				//colorbuffer_gray1[c_y][c_x] = 255;
//			}
			for (int i = 0 ; i < buf_edg[total_edg].total_pel;i++)
			{
				int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
				int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
				colorbuffer_gray1[c_y][c_x] = 255;
			}
			total_edg++;
		}
		else
		{
			buf_edg[total_edg].total_pel = 0;
			buf_edg[total_edg].other_pel = 0;
			buf_pel_find[l].removed = true;
		}
		draw_total_pels -= buf_edg[total_edg].total_pel;
//		if (draw_total_pels <= 5)
//			find = true;
	}
	
	return 0;
}
int draw_sports_new(byte **colorbuffer_gray1,int total_find,int max_area,int min_area)
{
	bool find = false;
	int draw_total_pels = total_find;
	int l = 0;
	int top_y = 0;
	int top_x = 0;
	int check_y = 0;
	int check_x = 0;
	int origin_x = 0;
	int origin_y = 0;
	while (!find)
	{
	
		for ( l = 0 ; l < total_find ; l++)
		{
			if (!buf_pel_find[l].removed)
			{
				//buf_pel_find[l].removed = true;
				check_x = buf_pel_find[l].x_loc;
				check_y = buf_pel_find[l].y_loc;
				break;
			}
		}

		if (l >= total_find - 1)
			find = true;
		while (check_pel(++check_y,check_x));

		origin_x = check_x;
		origin_y = --check_y;

		printf("origin_x: %d origin_y: %d\n",origin_x,origin_y);
	
		int find1 = find_edg(origin_x,origin_y,check_y,check_x,7);
		printf("find: %d\n",find1);

		if(cal_edg(&buf_edg[total_edg],total_find,max_area,min_area))
		{
//			for (int i = 0 ; i < buf_edg[total_edg].other_pel;i++)
//			{
//				int c_y = buf_edg[total_edg].others[i].y_loc;
//				int c_x = buf_edg[total_edg].others[i].x_loc;
//				//colorbuffer_gray1[c_y][c_x] = 255;
//			}
			for (int i = 0 ; i < buf_edg[total_edg].total_pel;i++)
			{
				int c_y = buf_edg[total_edg].edg_pel[i].y_loc;
				int c_x = buf_edg[total_edg].edg_pel[i].x_loc;
				colorbuffer_gray1[c_y][c_x] = 255;
			}
			total_edg++;
		}
		else
		{
			buf_edg[total_edg].total_pel = 0;
			buf_edg[total_edg].other_pel = 0;
			buf_pel_find[l].removed = true;
		}
		draw_total_pels -= buf_edg[total_edg].total_pel;
//		if (draw_total_pels <= 5)
//			find = true;
	}
	
	return 0;
}

unsigned char diff_level[256] = 
{
	0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,
	8,8,8,8,8,8,8,8,8,8,
	9,9,9,9,9,9,9,9,9,9,
	10,10,10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,
	12,12,12,12,12,12,12,12,12,12,
	13,13,13,13,13,13,13,13,13,13,
	14,14,14,14,14,14,14,14,14,14,
	15,15,15,15,15,15,15,15,15,15,
	16,16,16,16,16,16,16,16,16,16,
	17,17,17,17,17,17,17,17,17,17,
	18,18,18,18,18,18,18,18,18,18,
	19,19,19,19,19,19,19,19,19,19,
	20,20,20,20,20,20,20,20,20,20,
	21,21,21,21,21,21,21,21,21,21,
	22,22,22,22,22,22,22,22,22,22,
	23,23,23,23,23,23,23,23,23,23,
	24,24,24,24,24,24,24,24,24,24,
	24,24,24,24,24,24
};

int evenness_ana(byte **colorbuffer_gray,int *spX,int *spY,
				   INT_PTR nPointCount,int width,int height,float *result1)
{
	int j,i;
	int total_pels = 0;
	int temp_width = width;
	DWORD evenness = 0;
	int average = 0;
	int max = 0;
	int tmp0;
	int level = 15;
	byte **in_01;
	get_mem2D(&in_01,height,width);
	memset(result1,0,sizeof(float)*width);
	in_out(spX,spY,(int)nPointCount,width,height,in_01);
	for (j = 0;j < height;j++)
		for (i = 0;i < width;i++)
		{
			if (in_01[j][i]==1)
			{	
				colorbuffer_gray[j][i] = -1;
			}
		}
	for (j = 0;j < width;j++)
	{
		result1[j] = 0;
		int temp_height = 0;
		
		for (i = 0;i < height;i++)
		{
			if (colorbuffer_gray[i][j]>0&&colorbuffer_gray[i][j]<255)
			{
				result1[j]+=colorbuffer_gray[i][j];
				temp_height++;
			}
		}
		total_pels+=temp_height;
		if (temp_height)
			result1[j]/=temp_height;
		else
			temp_width--;
	}
	for (j = 0;j < width;j++)
		average+=(int)(result1[j]);
	if(temp_width>0) // changed by lou 090320
		average /= temp_width;
	for (j = 0;j < height;j++)
	{
		for(i = 0;i < width;i++)
		{
			if (colorbuffer_gray[j][i]>=0&&colorbuffer_gray[j][i]<255)
			{
				tmp0 = abs(colorbuffer_gray[j][i]-average);
				evenness+=tmp0*tmp0;
				tmp0 = tmp0 > 255?255:tmp0;
				max = tmp0 > max?tmp0:max;
				colorbuffer_gray[j][i] = tmp0;
			}
		}
	}
	max = max < level ?level:max;
	if(level>0) // changed by lou 090320
		max /= level;
	for (j = 0;j < height;j++)
	{
		for(i = 0;i < width;i++)
		{
			if (colorbuffer_gray[j][i]>=0&&colorbuffer_gray[j][i]<255)
			{
				tmp0 = colorbuffer_gray[j][i] / max;
				colorbuffer_gray[j][i] = tmp0 > level?level:tmp0;
			}
		}
	}
	if(total_pels>0) // changed by lou 090320
		evenness /= total_pels;
	evenness = (DWORD)sqrt(float(evenness));
	float tmp1 = float (evenness)/128;
	evenness = (DWORD)(100*tmp1);
	free_mem2Dbyte(in_01);
	return evenness;
}

int contrast(byte** colorbuffer_gray,int width,int height,int val)
{
	int tmp_c = 0;
	for (int i = 0;i < height;i++)
	{
		for (int j = 0; j < width;j++)
		{
			tmp_c=colorbuffer_gray[i][j];
			tmp_c*=(100+val);
			tmp_c/=100;
			tmp_c=tmp_c>255?255:tmp_c;
			colorbuffer_gray[i][j]=byte(tmp_c);
		}
	}
	return 0;
}
int winkel_line(byte **pGray,byte **in_01,int nWidth,int nHeight)
{
	int j,i;
	//vertical
	for (i = 0;i < nWidth;i++ )
	{
		for (j = 0;j < nHeight;j++)
		{
			if ( in_01[j][i] == 255)
			{
				int tmp_count =0;
				while(in_01[j++][i] == 255)
				{
					tmp_count++;
					if (j == nHeight-1)
						break;
				}
				pGray[j - tmp_count/2][i] =255;
//				pGray[j - tmp_count/2-1][i] =255;
//				pGray[j - tmp_count/2+1][i] =255;
			}
		}
	}

	//erase one loop
//	int loop = 1;
//	while (loop--)
//	{
//		for ( j = 0;j < nHeight;j++)
//			for (int i = 0;i < nWidth;i++)
//			{	
//				in_01[j][i] = 0;
//				in_01[j][i] = pGray[j][i];
//				pGray[j][i] = 0;
//			}
//		for (i = 1;i < nWidth-1;i++ )
//		{
//			for (j = 1;j < nHeight-1;j++)
//			{
//				if (in_01[j][i] == 255)
//				{
//					int tmp_count_sec = 0;
//					if (in_01[j-1][i-1] == 255)
//						tmp_count_sec++;
//					if (in_01[j-1][i+1] == 255)
//						tmp_count_sec++;
//					if (in_01[j-1][i] == 255)
//						tmp_count_sec++;
//					if (in_01[j][i-1] == 255)
//						tmp_count_sec++;
//					if (in_01[j][i+1] == 255)
//						tmp_count_sec++;
//					if (in_01[j+1][i-1] == 255)
//						tmp_count_sec++;
//					if (in_01[j+1][i] == 255)
//						tmp_count_sec++;
//					if (in_01[j+1][i+1] == 255)
//						tmp_count_sec++;
//					if (tmp_count_sec>0)
//						pGray[j][i] = 255;
//				}
//			}
//		}
//	}
	//link
	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
		{	
			in_01[j][i] = 0;
			in_01[j][i] = pGray[j][i];
			pGray[j][i] = 0;
		}
	for (i = 2;i < nWidth-2;i++ )
	{
		for (j = 2;j < nHeight-2;j++)
		{
			if(in_01[j][i] == 255)
			{
				pGray[j][i] = 255;
				int one_tmp_count = 0;
				//check range one
				if (in_01[j-1][i-1] == 255)
					one_tmp_count++;
				if (in_01[j-1][i] == 255)
					one_tmp_count++;
				if (in_01[j-1][i+1] == 255)
					one_tmp_count++;
				if (in_01[j][i-1] == 255)
					one_tmp_count++;
				if (in_01[j][i+1] == 255)
					one_tmp_count++;
				if (in_01[j+1][i-1] == 255)
					one_tmp_count++;
				if (in_01[j+1][i] == 255)
					one_tmp_count++;
				if (in_01[j+1][i+1] == 255)
					one_tmp_count++;
				if (one_tmp_count>1)
					continue;

				//check range two
				if (in_01[j-2][i-2] == 255)
				{
					pGray[j-2][i-2] = 255;
					pGray[j-1][i-1] = 255;
				}
				if (in_01[j-2][i-1] == 255)
				{
					pGray[j-2][i-1] = 255;
					pGray[j-1][i-1] = 255;
				}
				if (in_01[j-2][i] == 255)
				{
					pGray[j-2][i] = 255;
					pGray[j-1][i] = 255;
				}
				if (in_01[j-2][i+1] == 255)
				{
					pGray[j-2][i+1] = 255;
					pGray[j-1][i+1] = 255;
				}
				if (in_01[j-2][i+2] == 255)
				{
					pGray[j-2][i+2] = 255;
					pGray[j-1][i+1] = 255;
				}
				if (in_01[j-1][i-2] == 255)
				{
					pGray[j-1][i-2] = 255;
					pGray[j-1][i-1] = 255;
				}
				if (in_01[j-1][i+2] == 255)
				{
					pGray[j-1][i+2] = 255;
					pGray[j-1][i+1] = 255;
				}
				if (in_01[j][i-2] == 255)
				{
					pGray[j][i-2] = 255;
					pGray[j][i-1] = 255;
				}
				if (in_01[j][i+2] == 255)
				{
					pGray[j][i+2] = 255;
					pGray[j][i+1] = 255;
				}
				if (in_01[j+1][i-2] == 255)
				{
					pGray[j+1][i-2] = 255;
					pGray[j+1][i-1] = 255;
				}
				if (in_01[j+1][i+2] == 255)
				{
					pGray[j+1][i+2] = 255;
					pGray[j+1][i+1] = 255;
				}
				if (in_01[j+2][i-2] == 255)
				{
					pGray[j+2][i-2] = 255;
					pGray[j+1][i-1] = 255;
				}
				if (in_01[j+2][i-1] == 255)
				{
					pGray[j+2][i-1] = 255;
					pGray[j+1][i-1] = 255;
				}
				if (in_01[j+2][i] == 255)
				{
					pGray[j+2][i] = 255;
					pGray[j+1][i] = 255;
				}
				if (in_01[j+2][i+1] == 255)
				{
					pGray[j+2][i+1] = 255;
					pGray[j+1][i+1] = 255;
				}
				if (in_01[j+2][i+2] == 255)
				{
					pGray[j+2][i+2] = 255;
					pGray[j+1][i+1] = 255;
				}
			}
		}
	}


	return 0;
}

int winkel_kuo(byte **pGray,byte **in_01,byte **orig,int width,int height,int yu)
{
	int j,i,current_value,abs_value;
	
	for (j = 0;j < height ;j++)
			for (i = 0;i < width;i++)
			{
				in_01[j][i] = pGray[j][i];
			}

	for (j = 5;j < height - 5;j++)
		for (i = 5;i < width - 5;i++)
		{
			if (in_01[j][i] == 255)
			{
				current_value = orig[j][i];
				
				abs_value = abs(current_value - orig[j-1][i-1]);
				if (abs_value < yu)
					pGray[j-1][i-1] = 255;

				abs_value = abs(current_value - orig[j-1][i]);
				if (abs_value < yu)
					pGray[j-1][i] = 255;
				
				abs_value = abs(current_value - orig[j-1][i+1]);
				if (abs_value < yu)
					pGray[j-1][i+1] = 255;

				abs_value = abs(current_value - orig[j][i-1]);
				if (abs_value < yu)
					pGray[j][i-1] = 255;

				abs_value = abs(current_value - orig[j][i]);
				if (abs_value < yu)
					pGray[j][i] = 255;
				
				abs_value = abs(current_value - orig[j][i+1]);
				if (abs_value < yu)
					pGray[j][i+1] = 255;

				abs_value = abs(current_value - orig[j+1][i-1]);
				if (abs_value < yu)
					pGray[j+1][i-1] = 255;

				abs_value = abs(current_value - orig[j+1][i]);
				if (abs_value < yu)
					pGray[j+1][i] = 255;
				
				abs_value = abs(current_value - orig[j+1][i+1]);
				if (abs_value < yu)
					pGray[j+1][i+1] = 255;
			}
		}

		for (j = 0;j < height ;j++)
			for (i = 0;i < width;i++)
			{
				in_01[j][i] = pGray[j][i];
			}

		return 0;
}

int winkel_ana_new1(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
					  INT_PTR nPointCount,int *spArea,int nMin,int nMax,
					  int max_area,int min_area,int con_val,HWND hWnd=NULL)
{
	byte **in_01;
	byte **pGray1;
//	byte **pGray4;
//	byte **pGray4_1;
//	int nHeight4,nWidth4;
//	nWidth4 = nWidth%2 == 0 ? (nWidth/2 -1):(nWidth/2);
//	nHeight4 = nHeight%2 == 0 ? (nHeight/2 -1):(nHeight/2);
	get_mem2D(&in_01,nHeight,nWidth);
	get_mem2D(&pGray1,nHeight,nWidth);
//	get_mem2D(&pGray4,nHeight4,nWidth4);
//	get_mem2D(&pGray4_1,nHeight4,nWidth4);
	in_out(spX,spY,(int)nPointCount,nWidth,nHeight,in_01);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,10,0);
	int avg_tmp = 0;
	init_face();
	int key_value = 0;
	int key_count = 0;
	int spArea_in = 1;
	int j ,i;
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (in_01[j][i]==1)
			{	
				pGray[j][i] = 0;
			}
			else
			{
				avg_tmp+=pGray[j][i];
				spArea_in++;
			}
		}
	}
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			pGray1[j][i] = pGray[j][i];
		}
	}
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,20,0);
	*spArea = spArea_in;
	avg_tmp/=spArea_in;
	//winkel_resize(pGray,pGray4,nWidth4,nHeight4);

	//contrast(pGray1,nWidth,nHeight,con_val);
	int pels_find;
	int tmpmax = nMax;
	avg_tmp*=(100+con_val);
	avg_tmp/=100;
	avg_tmp = avg_tmp > 255?255:avg_tmp;
	nMax*=(100+con_val);
	nMax/=100;
	//nMin +=(nMax - tmpmax);
	//lowfilter(pGray1,pGray,nWidth,nHeight);
	pels_find = face_winkel_ana1(nMin,nMax,nWidth,nHeight,pGray,avg_tmp,hWnd);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,40,0);
	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
		{
			//in_01 = 0;
			pGray[j][i] = 0;
		}

	for (i = 0;i < pels_find;i++ )
	{
		int x1 = buf_pel_find[i].x_loc;
		int y1 = buf_pel_find[i].y_loc;
		pGray[y1][x1] = 255;
		in_01[y1][x1] = 255;
	}
	erasepixel(pGray,nWidth,nHeight,1);
	erasepixel(pGray,nWidth,nHeight,1);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,50,0);	
//	winkel_kuo(pGray,in_01,pGray1,nWidth,nHeight,10);
	thinning(pGray,nWidth,nHeight); 
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,60,0);
// 	erasepixel(pGray,nWidth,nHeight,1);
	erasepixel(pGray,nWidth,nHeight,1);
	erasepixel(pGray,nWidth,nHeight,0);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,70,0);

//	winkel_kuo(pGray,in_01,pGray1,nWidth,nHeight,10);
	winkel_kuo(pGray,in_01,pGray1,nWidth,nHeight,10);
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,80,0);
	thinning(pGray,nWidth,nHeight); 
	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,90,0);
	erasepixel(pGray,nWidth,nHeight,0);

//	winkel_kuo(pGray,in_01,pGray1,nWidth,nHeight,10);
//	winkel_kuo(pGray,in_01,pGray1,nWidth,nHeight,10);
//	thinning(pGray,nWidth,nHeight); 
//	erasepixel(pGray,nWidth,nHeight,0);

//	winkel_line(pGray,in_01,nWidth,nHeight);
	pels_find = 0;
	//init the check buffer
	for ( j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (pGray[j][i] == 255)
			{
				buf_pel_find[pels_find].x_loc = i;
				buf_pel_find[pels_find].y_loc = j;
				buf_pel_find[pels_find].removed = false;
				buf_pel_log[j][i].n_pel_find = pels_find;
				buf_pel_log[j][i].right_pel = true;
				pels_find++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
	}

	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;

	draw_winkel_new(pGray,pels_find,max_area,min_area);

	LibFA_SendMessage(hWnd,WM_USER_ANALYSEPROCESS,100,0);
	free_mem2Dbyte(in_01);
	free_mem2Dbyte(pGray1);
//	free_mem2Dbyte(pGray4);
//	free_mem2Dbyte(pGray4_1);
	return total_edg;

}
int winkel_ana_new(byte **pGray,int nWidth,int nHeight,int *spX,int *spY,
					  INT_PTR nPointCount,int *spArea,int nMin,int nMax,
					  int max_area,int min_area,int con_val,HWND hWnd=NULL)
{
	byte **in_01;
	get_mem2D(&in_01,nHeight,nWidth);
	in_out(spX,spY,(int)nPointCount,nWidth,nHeight,in_01);
	int avg_tmp = 0;
	init_face();
	int spArea_in = 1;
	int j ,i;
	for (j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (in_01[j][i]==1)
			{	
				pGray[j][i] = 0;
			}
			else
			{
				avg_tmp+=pGray[j][i];
				spArea_in++;
			}
		}
	}
	*spArea = spArea_in;
	avg_tmp/=spArea_in;
	contrast(pGray,nWidth,nHeight,con_val);
	int pels_find;
	int tmpmax = nMax;
	avg_tmp*=(100+con_val);
	avg_tmp/=100;
	avg_tmp = avg_tmp > 255?255:avg_tmp;
	nMax*=(100+con_val);
	nMax/=100;
	//nMin +=(nMax - tmpmax);
	pels_find = face_winkel_ana(nMin,nMax,nWidth,nHeight,pGray,avg_tmp,hWnd);

	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;

	for (i = 0;i < pels_find;i++ )
	{
		int x1 = buf_pel_find[i].x_loc;
		int y1 = buf_pel_find[i].y_loc;
		in_01[y1][x1] = 255;
	}
	
	winkel_line(pGray,in_01,nWidth,nHeight);
	pels_find = 0;
	//init the check buffer
	for ( j = 0;j < nHeight;j++)
	{
		for (int i = 0;i < nWidth;i++)
		{
			if (pGray[j][i] == 255)
			{
				buf_pel_find[pels_find].x_loc = i;
				buf_pel_find[pels_find].y_loc = j;
				buf_pel_find[pels_find].removed = false;
				buf_pel_log[j][i].n_pel_find = pels_find;
				buf_pel_log[j][i].right_pel = true;
				pels_find++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
	}

	for ( j = 0;j < nHeight;j++)
		for (int i = 0;i < nWidth;i++)
			pGray[j][i] = 0;

	draw_winkel_new(pGray,pels_find,max_area,min_area);
	
	free_mem2Dbyte(in_01);
	return total_edg;
}

int face_winkel_ana(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd=NULL)
{
	
	int pel = 0;
	max_val = avg_all*min_val/100;
	min_val = 10;
	for (int j = 0 ; j < height ; j++)
		for (int i = 0 ; i < width*5/7 ; i++)
		{
			if (colorbuffer_gray1[j][i]<=max_val&&colorbuffer_gray1[j][i]>=min_val)
			{
				//colorbuffer_r1[j][i] = colorbuffer_g1[j][i] = colorbuffer_b1[j][i] = 0;
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				buf_pel_find[pel].removed = false;
				buf_pel_log[j][i].n_pel_find = pel;
				buf_pel_log[j][i].right_pel = true;
				pel++;
			}
			else
			{
				buf_pel_log[j][i].right_pel = false;
				buf_pel_log[j][i].n_pel_find = 0;
			}
		}
	return pel;
}

void Differential(byte** image_in,byte** image_out,int xsize,int ysize,float amp)
{
	static int cx[9] = { 0, 0, 0,
						 0, 1, 0,
						 0, 0,-1};
	static int cy[9] = { 0, 0, 0,
						 0, 0, 1,
						 0, -1,0};
	int d[9];
	int i,j,dat;
	float xx,yy,zz;

	for (j = 50;j < ysize-50;j++)
	{
		for (i = 50;i < xsize-50;i++)
		{
			d[0] = image_in[j-1][i-1];
			d[1] = image_in[j-1][i];
			d[2] = image_in[j-1][i+1];
			d[3] = image_in[j][i-1];
			d[4] = image_in[j][i];
			d[5] = image_in[j][i+1];
			d[6] = image_in[j+1][i-1];
			d[7] = image_in[j+1][i];
			d[8] = image_in[j+1][i+1];

			xx = (float)(cx[0]*d[0]+cx[1]*d[1]+cx[2]*d[2]+cx[3]*d[3]+cx[4]*d[4]+
						cx[5]*d[5]+cx[6]*d[6]+cx[7]*d[7]+cx[8]*d[8]);
			yy = (float)(cy[0]*d[0]+cy[1]*d[1]+cy[2]*d[2]+cy[3]*d[3]+cy[4]*d[4]+
						cy[5]*d[5]+cy[6]*d[6]+cy[7]*d[7]+cy[8]*d[8]);
			zz = (float)(amp*sqrt(xx*xx+yy*yy));
			dat = (int) zz;
			if (dat > 255) dat = 255;
			image_out[j][i] = dat;
		}
	}
}

#define Clip3(min,max,val) (((val)<(min))?(min):(((val)>(max-1))?(max-1):(val)))
int face_winkel_ana1(int min_val,int max_val,int width,int height,byte **colorbuffer_gray1,int avg_all,HWND hWnd=NULL)
{
	
	int pel = 0;
	int count_value;
	bool ver_check ;
	bool hor_check ;
	int up_check;
	int down_check;
	int left_check;
	int right_check;
	int check_16,check_14,check_12,check_10,check_8,check_6,check_4,check_2;

	for (int j = 0 ; j < height ; j++)
		for (int i = 0 ; i < width ; i++)
		{
			ver_check = false;
			hor_check = false;
			up_check = 0;
			down_check = 0;
			left_check = 0;
			right_check = 0;
			count_value = colorbuffer_gray1[j][i];
			//check up
			check_16 = colorbuffer_gray1[Clip3(0,height,j-16)][i];
			check_14 = colorbuffer_gray1[Clip3(0,height,j-14)][i];
			check_12 = colorbuffer_gray1[Clip3(0,height,j-12)][i];
			check_10 = colorbuffer_gray1[Clip3(0,height,j-10)][i];
			check_8 = colorbuffer_gray1[Clip3(0,height,j-8)][i];
			check_6 = colorbuffer_gray1[Clip3(0,height,j-6)][i];
			check_4 = colorbuffer_gray1[Clip3(0,height,j-4)][i];
			check_2 = colorbuffer_gray1[Clip3(0,height,j-2)][i];
			if (check_2 > count_value)
				up_check++;
			if (check_4 > count_value)
				up_check++;
			if (check_6 > count_value)
				up_check++;
			if (check_8 > count_value)
				up_check++;
			if (check_10 > count_value)
				up_check++;
			if (check_12 > count_value)
				up_check++;
			if (check_14 > count_value)
				up_check++;
			if (check_16 > count_value)
				up_check++;
			
			//check down
			check_16 = colorbuffer_gray1[Clip3(0,height,j+16)][i];
			check_14 = colorbuffer_gray1[Clip3(0,height,j+14)][i];
			check_12 = colorbuffer_gray1[Clip3(0,height,j+12)][i];
			check_10 = colorbuffer_gray1[Clip3(0,height,j+10)][i];
			check_8 = colorbuffer_gray1[Clip3(0,height,j+8)][i];
			check_6 = colorbuffer_gray1[Clip3(0,height,j+6)][i];
			check_4 = colorbuffer_gray1[Clip3(0,height,j+4)][i];
			check_2 = colorbuffer_gray1[Clip3(0,height,j+2)][i];
			if (check_2 > count_value)
				down_check++;
			if (check_4 > count_value)
				down_check++;
			if (check_6 > count_value)
				down_check++;
			if (check_8 > count_value)
				down_check++;
			if (check_10 > count_value)
				down_check++;
			if (check_12 > count_value)
				down_check++;
			if (check_14 > count_value)
				down_check++;
			if (check_16 > count_value)
				down_check++;

			//deside ver
			if (up_check > 5 && down_check > 5)
				ver_check = true;

			//check left
			check_16 = colorbuffer_gray1[j][Clip3(0,width,i-16)];
			check_14 = colorbuffer_gray1[j][Clip3(0,width,i-14)];
			check_12 = colorbuffer_gray1[j][Clip3(0,width,i-12)];
			check_10 = colorbuffer_gray1[j][Clip3(0,width,i-10)];
			check_8 = colorbuffer_gray1[j][Clip3(0,width,i-8)];
			check_6 = colorbuffer_gray1[j][Clip3(0,width,i-6)];
			check_4 = colorbuffer_gray1[j][Clip3(0,width,i-4)];
			check_2 = colorbuffer_gray1[j][Clip3(0,width,i-2)];
			if (check_2 > count_value)
				left_check++;
			if (check_4 > count_value)
				left_check++;
			if (check_6 > count_value)
				left_check++;
			if (check_8 > count_value)
				left_check++;
			if (check_10 > count_value)
				left_check++;
			if (check_12 > count_value)
				left_check++;
			if (check_14 > count_value)
				left_check++;
			if (check_16 > count_value)
				left_check++;

			//check right
			check_16 = colorbuffer_gray1[j][Clip3(0,width,i+16)];
			check_14 = colorbuffer_gray1[j][Clip3(0,width,i+14)];
			check_12 = colorbuffer_gray1[j][Clip3(0,width,i+12)];
			check_10 = colorbuffer_gray1[j][Clip3(0,width,i+10)];
			check_8 = colorbuffer_gray1[j][Clip3(0,width,i+8)];
			check_6 = colorbuffer_gray1[j][Clip3(0,width,i+6)];
			check_4 = colorbuffer_gray1[j][Clip3(0,width,i+4)];
			check_2 = colorbuffer_gray1[j][Clip3(0,width,i+2)];
			if (check_2 > count_value)
				right_check++;
			if (check_4 > count_value)
				right_check++;
			if (check_6 > count_value)
				right_check++;
			if (check_8 > count_value)
				right_check++;
			if (check_10 > count_value)
				right_check++;
			if (check_12 > count_value)
				right_check++;
			if (check_14 > count_value)
				right_check++;
			if (check_16 > count_value)
				right_check++;

			//deside hor
			if (left_check > 5 && right_check > 5)
				hor_check = true;

			if (ver_check&&hor_check)
			{
				int x,y;
				int full = 0;
				for (y=j-2;y<j+3;y++)
					for (x=i-2;x<i+3;x++)
					{
						int count_abs = abs(colorbuffer_gray1[Clip3(0,height,y)][Clip3(0,width,x)]-count_value);
						if (count_abs < 15)
							full++;
					}
				if (full < 30)
				{
					buf_pel_find[pel].x_loc = i;
					buf_pel_find[pel].y_loc = j;
					pel++;
				}
				else
					colorbuffer_gray1[j][i] = 0;
			}
			else if (ver_check)
			{
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				pel++;
			}
			else if (hor_check)
			{
				buf_pel_find[pel].x_loc = i;
				buf_pel_find[pel].y_loc = j;
				pel++;
			}
			else
			{
				;
			}

		}
	return pel;
}

bool thinning(byte** pGray,int width,int height)
{
	byte** pGray_thin;
	bool bModified;			//????	
	long i,j,m,n;			//???????

	//???????
	bool bCondition1;
	bool bCondition2;
	bool bCondition3;
	bool bCondition4;
	
	byte nCount;	//??????	
	byte pixel;	//?????	
	byte neighbour[5][5];	//5??5?????????????

	// ????????????????????
	get_mem2D(&pGray_thin,height,width);
	
	bModified = true;
	while(bModified)
	{
		bModified = false;
		
		for(j = 2; j <height-2; j++)
		{
			for(i = 2;i <width-2; i++)
			{
				bCondition1 = false;
				bCondition2 = false;
				bCondition3 = false;
				bCondition4 = false;

				//???????5??5???????????????????????????????????????

				pixel = pGray[j][i];

				//???????????0??255???????????
				if(pixel != 255 )
				{
					pGray_thin[j][i] = 0;
					continue;
				}
				
				//??�?????????5??5??????????????????0???????????1????
				for (m = -2;m < 3;m++ )
				{
					for (n = -2;n < 3;n++)
					{
						if(pGray[j+m][i+n] == 255)
							neighbour[m+2][n+2] = 1;
						else
							neighbour[m+2][n+2] = 0;
					}
				}

				//????????????
				//???2<=NZ(P1)<=6
				nCount =  neighbour[1][1] + neighbour[1][2] + neighbour[1][3] \
						+ neighbour[2][1] + neighbour[2][3] + \
						+ neighbour[3][1] + neighbour[3][2] + neighbour[3][3];
				if ( nCount >= 2 && nCount <=6)
				{
					bCondition1 = true;
				}

				//???Z0(P1)=1
				nCount = 0;
				if (neighbour[1][2] == 0 && neighbour[1][1] == 1)
					nCount++;
				if (neighbour[1][1] == 0 && neighbour[2][1] == 1)
					nCount++;
				if (neighbour[2][1] == 0 && neighbour[3][1] == 1)
					nCount++;
				if (neighbour[3][1] == 0 && neighbour[3][2] == 1)
					nCount++;
				if (neighbour[3][2] == 0 && neighbour[3][3] == 1)
					nCount++;
				if (neighbour[3][3] == 0 && neighbour[2][3] == 1)
					nCount++;
				if (neighbour[2][3] == 0 && neighbour[1][3] == 1)
					nCount++;
				if (neighbour[1][3] == 0 && neighbour[1][2] == 1)
					nCount++;
				if (nCount == 1)
					bCondition2 = true;

				//???P2*P4*P8=0 or Z0(p2)!=1
				if (neighbour[1][2]*neighbour[2][1]*neighbour[2][3] == 0)
				{
					bCondition3 = TRUE;
				}
				else
				{
					nCount = 0;
					if (neighbour[0][2] == 0 && neighbour[0][1] == 1)
						nCount++;
					if (neighbour[0][1] == 0 && neighbour[1][1] == 1)
						nCount++;
					if (neighbour[1][1] == 0 && neighbour[2][1] == 1)
						nCount++;
					if (neighbour[2][1] == 0 && neighbour[2][2] == 1)
						nCount++;
					if (neighbour[2][2] == 0 && neighbour[2][3] == 1)
						nCount++;
					if (neighbour[2][3] == 0 && neighbour[1][3] == 1)
						nCount++;
					if (neighbour[1][3] == 0 && neighbour[0][3] == 1)
						nCount++;
					if (neighbour[0][3] == 0 && neighbour[0][2] == 1)
						nCount++;
					if (nCount != 1)
						bCondition3 = true;
				}

				//???P2*P4*P6=0 or Z0(p4)!=1
				if (neighbour[1][2]*neighbour[2][1]*neighbour[3][2] == 0)
				{
					bCondition4 = true;
				}
				else
				{
					nCount = 0;
					if (neighbour[1][1] == 0 && neighbour[1][0] == 1)
						nCount++;
					if (neighbour[1][0] == 0 && neighbour[2][0] == 1)
						nCount++;
					if (neighbour[2][0] == 0 && neighbour[3][0] == 1)
						nCount++;
					if (neighbour[3][0] == 0 && neighbour[3][1] == 1)
						nCount++;
					if (neighbour[3][1] == 0 && neighbour[3][2] == 1)
						nCount++;
					if (neighbour[3][2] == 0 && neighbour[2][2] == 1)
						nCount++;
					if (neighbour[2][2] == 0 && neighbour[1][2] == 1)
						nCount++;
					if (neighbour[1][2] == 0 && neighbour[1][1] == 1)
						nCount++;
					if (nCount != 1)
						bCondition4 = true;
				}
				if(bCondition1 && bCondition2 && bCondition3 && bCondition4)
				{
					pGray_thin[j][i] = 0;
					bModified = true;
				}
				else
				{
					pGray_thin[j][i] = 255;
				}
			}
		}
		// ????????????
		for(j = 2; j <height-2; j++)
			for(i = 2;i <width-2; i++)
			{
				pGray[j][i] = pGray_thin[j][i];
			}
	}

	free_mem2Dbyte(pGray_thin);	

	return true;
}

bool erasepixel(byte** pGray,int width,int height,int n)
{
	byte** pGray_erase;
	int j,i;
	byte pixel;
	get_mem2D(&pGray_erase,height,width);

	for (j = 1; j < height-1; j++)
		for (i = 1; i < width-1; i++)
		{
			pixel = pGray[j][i];
			if (pixel == 255)
			{
				int ncount = 0;
				if (pGray[j-1][i-1] == 255)
					ncount++;
				if (pGray[j-1][i] == 255)
					ncount++;
				if (pGray[j-1][i+1] == 255)
					ncount++;
				if (pGray[j][i-1] == 255)
					ncount++;
				if (pGray[j][i+1] == 255)
					ncount++;
				if (pGray[j+1][i-1] == 255)
					ncount++;
				if (pGray[j+1][i] == 255)
					ncount++;
				if (pGray[j+1][i+1] == 255)
					ncount++;
				if ( ncount > n)
					pGray_erase[j][i] = 255;
				else
					pGray_erase[j][i] = 0;
			}
			else
				pGray_erase[j][i] = 0;
		}

	for (j = 0; j < height; j++)
		for (i = 0; i < width; i++)
		{
			pGray[j][i] = pGray_erase[j][i];
		}
	
	free_mem2Dbyte(pGray_erase);	

	return true;
}