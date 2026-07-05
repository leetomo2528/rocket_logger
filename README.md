# rocket_logger — 로켓 발사 데이터 로거

Arduino Nano 33 BLE Rev2 + ±400g 가속도계 + MicroSD 카드로 비행 중 3축 가속도/자이로/온습도 CSV 로깅.

## 보드
- Arduino Nano 33 BLE Rev2 (3.3V 전용)
- H3LIS331DL (SEN-14480) ±400g 고중력 가속도계
- Micro SD 모듈 (3.3V SPI)

## 센서 구성
| 센서 | 범위 | 용도 |
|------|------|------|
| BMI270 (내장) | ±16g | 일반 비행 가속도, 자이로 |
| H3LIS331DL (SEN-14480) | ±400g | 발사 충격, 고중력 이벤트 |
| HS300x (내장) | — | 온도 / 습도 |
| Micro SD 모듈 | — | CSV 데이터 저장 |

## 배선

### SD 카드 모듈 (SPI, 3.3V 모듈 필수)
| SD 모듈 | Nano 33 BLE |
|---------|-------------|
| CS  | D10 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |
| VCC | 3.3V |
| GND | GND |

### H3LIS331DL (I2C)
| H3LIS331DL | Nano 33 BLE |
|------------|-------------|
| SDA | SDA (A4) |
| SCL | SCL (A5) |
| VCC | 3.3V |
| GND | GND |
| SA0 | GND → 주소 0x18 |

## 필요 라이브러리
- `Arduino_BMI270_BMM150` (라이브러리 매니저)
- `Arduino_HS300x` (라이브러리 매니저)
- `SparkFun LIS331` (라이브러리 매니저)
- `SD` (내장)

## CSV 컬럼 구조
```
time_ms,
ax_g, ay_g, az_g,          ← BMI270 (±16g)
gx_dps, gy_dps, gz_dps,
vx_ms, vy_ms, vz_ms, speed_ms,
hax_g, hay_g, haz_g,        ← H3LIS331DL (±400g)
hspeed_ms,
temp_C, humidity_pct
```

## 동작
- 100ms 샘플링 (10Hz). 비행 시 `SAMPLE_MS=20` (50Hz) 권장.
- 파일명 자동 증분: `data00.csv` ~ `data99.csv` (재시작 시 덮어쓰기 방지)
- 10샘플마다 SD flush
- 시리얼 모니터 (115200) 동시 출력

## 비고
- 속도 적분은 중력 포함 → 정지 상태에서 드리프트 발생. 발사 감지 시점에 `vx=vy=vz=0` 리셋 권장.
- BMI270 포화(±16g 초과) 시 H3LIS331DL 데이터로 보완.
- 비행 중 샘플링 높이려면 `#define SAMPLE_MS 20` (50Hz) 변경.

## 출처
`~/Documents/yunje/Dev/Arduino Nano 33 BLE - 로켓 데이터 로거.md` 의 코드를 `.ino`로 분리 저장.