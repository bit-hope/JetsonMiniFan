#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
	int16_t minTemperature;
	int16_t maxTemperature;	
	uint8_t minSpeedPercent;
	uint8_t maxSpeedPercent;
	uint8_t checkIntervalSeconds;
} stConfig;

void
ReadConfig(stConfig* _cfg)
{
	memset(_cfg, 0, sizeof(*_cfg));
	// default configurations.
	_cfg->minTemperature = 32;
	_cfg->maxTemperature = 64;
	_cfg->minSpeedPercent = 8;
	_cfg->maxSpeedPercent = 100;
	_cfg->checkIntervalSeconds = 1;
	// you can read config from a file.
}

void
GenerateFactor(float* _factor_a, float* _factor_b, float* _factor_c, const stConfig* _cfg)
{
	const float _x_0 = (float)(_cfg->minTemperature);
	const float _x_1 = (float)(_cfg->maxTemperature);
	const float _y_0 = (float)(_cfg->minSpeedPercent);
	const float _y_1 = (float)(_cfg->maxSpeedPercent);
	const float _delta_x = _x_1 - _x_0;
	const float _delta_y = _y_1 - _y_0;
	// generate the quadratic function: f_0(x) = a_0 * x * x + b_0 * x + c_0.
	const float _a_0 = _delta_y / (_delta_x * _delta_x);
	const float _b_0 = 0.0f - (_a_0 * _x_0 * 2);
	const float _c_0 = (_b_0 * _b_0) / (_a_0 * 4) + _y_0;
	// generate the linear function: f_1(x) = b_1 * x + c_1.
	const float _b_1 = _delta_y / _delta_x;
	const float _c_1 = _y_0 - _b_1 * _x_0;
	// the quadratic function grows two slow while the linear function grows to fast.
	// so we mix the two function as one: f(x) = f_0(x) * mix_factor + f_1(x) * (1 - mix_factor).
	// the target is a new quadratic function: f(x) = a * x * x + b * x + c.
	const float _mixFactor = 0.75f;
	*_factor_a = _a_0 * _mixFactor;
	*_factor_b = _b_0 * _mixFactor + _b_1 * (1.0f - _mixFactor);
	*_factor_c = _c_0 * _mixFactor + _c_1 * (1.0f - _mixFactor);
}

uint8_t 
ConvertPercentToSpeedValue(float _percent)
{
	float _speed = _percent * 255 / 100 + 0.5f;
	return (_speed < 255.0f) ? (uint8_t)(_speed) : 255;
}

void
GenerateSpeedTable(uint8_t* _table, const stConfig* _cfg)
{
	// f(t) = min_speed, t < min_temperature.
	// f(t) = max_speed, t > max_temperature.
	// f(t) = a * t * t + b * t + c, (t >= min_temperature) && (t <= max_temperature).
	float _factor_a = 0.0f;
	float _factor_b = 0.0f;
	float _factor_c = 0.0f;
	GenerateFactor(&_factor_a, &_factor_b, &_factor_c, _cfg);

	_table[0] = ConvertPercentToSpeedValue(_cfg->minSpeedPercent);
	for (int16_t _temperature = (_cfg->minTemperature + 1); _temperature < _cfg->maxTemperature; ++_temperature)
	{
		float _percent = _factor_a * _temperature * _temperature + _factor_b * _temperature + _factor_c;
		_table[_temperature - _cfg->minTemperature] = ConvertPercentToSpeedValue(_percent);
	}
	_table[_cfg->maxTemperature - _cfg->minTemperature] = ConvertPercentToSpeedValue(_cfg->maxSpeedPercent);
}

int16_t
GetCurrentTemperature()
{
	FILE* _fp = fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "r");
	if (_fp == NULL)
	{
		return 0;
	}

	char _buffer[16 + sizeof(void*)]; 
	size_t _bytes = fread(_buffer, 1, 16, _fp);
	fclose(_fp);
	if (_bytes < 1)
	{
		return 0;
	}
	_buffer[_bytes] = '\0';

	char* _end = NULL;
	const long _value = strtol(_buffer, &_end, 10);
	return ((_end == NULL) || (_end == _buffer)) ? 0 : (int16_t)((_value + 999) / 1000);
}

uint8_t
GetFanSpeed()
{
	FILE* _fp = fopen("/sys/devices/pwm-fan/target_pwm", "r");
	if (_fp == NULL)
	{
		return 0;
	}

	char _buffer[8 + sizeof(void*)]; 
	size_t _bytes = fread(_buffer, 1, 8, _fp);
	fclose(_fp);
	if (_bytes < 1)
	{
		return 0;
	}
	_buffer[_bytes] = '\0';

	char* _end = NULL;
	const long _value = strtol(_buffer, &_end, 10);
	return ((_end == NULL) || (_end == _buffer)) ? 0 : (uint8_t)(_value);
}

void
SetFanSpeed(uint8_t _speed)
{
	FILE* _fp = fopen("/sys/devices/pwm-fan/target_pwm", "w");
	if (_fp != NULL)
	{
		fprintf(_fp, "%d\n", _speed);
		fclose(_fp);
	}
}

int
main()
{
	stConfig _cfg;
	ReadConfig(&_cfg);

	uint8_t _speedTable[64]; // 64 is enough for range [minTemperature, maxTemperature], or give a bigger one. 

	if (_cfg.minSpeedPercent < _cfg.maxSpeedPercent)
	{
		GenerateSpeedTable(_speedTable, &_cfg);
	}
	else
	{
		_speedTable[0] = ConvertPercentToSpeedValue(_cfg.minSpeedPercent);
	}

	// an optimization for reduce whine noise, while the fan switch between speed up and slow down frequently.
	uint8_t _tendToSlowDownTimes = 0;
	while (1)
	{
		uint8_t _nextSpeed = 0;
		if (_cfg.minSpeedPercent < _cfg.maxSpeedPercent)
		{
			const int16_t _temperature = GetCurrentTemperature();
			if (_temperature < _cfg.minTemperature)
			{
				_nextSpeed = _speedTable[0];
			}
			else if (_temperature > _cfg.maxTemperature)
			{
				_nextSpeed = _speedTable[_cfg.maxTemperature - _cfg.minTemperature];
			}
			else
			{
				_nextSpeed = _speedTable[_temperature - _cfg.minTemperature];
			}
		}
		else
		{
			_nextSpeed = _speedTable[0];
		}

		const uint8_t _currentSpeed = GetFanSpeed();

		if (_nextSpeed < _currentSpeed) // tend to slow down. 
		{
			// check the times, do not slow down immediately.
			if ((++_tendToSlowDownTimes) > 8) 
			{
				_tendToSlowDownTimes = 0;				
				SetFanSpeed(_nextSpeed);
			}
		}
		else if (_nextSpeed > _currentSpeed) // tend to speed up.
		{
			// just speed up immediately.
			_tendToSlowDownTimes = 0;
			SetFanSpeed(_nextSpeed);
		}
		else // keep speed.
		{
			_tendToSlowDownTimes = 0;
		}

		usleep(1000 * 1000 * _cfg.checkIntervalSeconds);
	}

	return 0;
}
