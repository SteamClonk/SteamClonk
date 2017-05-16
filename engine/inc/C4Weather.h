/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Controls temperature, wind, and natural disasters */

#ifndef INC_C4Weather
#define INC_C4Weather

class C4Weather
  {
  public:
    C4Weather();
	  ~C4Weather();
  public:
    int32_t Season,YearSpeed,SeasonDelay;
    int32_t Wind,TargetWind;
    int32_t Temperature,TemperatureRange,Climate;
    int32_t MeteoriteLevel,VolcanoLevel,EarthquakeLevel,LightningLevel;
		int32_t NoGamma;
  public:
	  void Default();
		void Clear();
    void Execute();
	  void SetClimate(int32_t iClimate);
	  void SetSeason(int32_t iSeason);
	  void SetTemperature(int32_t iTemperature);
    void Init(BOOL fScenario);
	  void SetWind(int32_t iWind);
		int32_t GetWind(int32_t x, int32_t y);
		int32_t GetTemperature();
	  int32_t GetSeason();
	  int32_t GetClimate();
		BOOL LaunchLightning(int32_t x, int32_t y, int32_t xdir, int32_t xrange, int32_t ydir, int32_t yrange, BOOL fDoGamma);
		BOOL LaunchVolcano(int32_t mat, int32_t x, int32_t y, int32_t size);
	  BOOL LaunchEarthquake(int32_t iX, int32_t iY);
	  BOOL LaunchCloud(int32_t iX, int32_t iY, int32_t iWidth, int32_t iStrength, const char *szPrecipitation);
		void SetSeasonGamma();		// set gamma adjustment for season
    void CompileFunc(StdCompiler *pComp);
  };

#endif
