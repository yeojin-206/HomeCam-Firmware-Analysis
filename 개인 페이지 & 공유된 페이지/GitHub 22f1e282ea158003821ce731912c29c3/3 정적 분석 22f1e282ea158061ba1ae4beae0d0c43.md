# 3. 정적 분석

### **시작하는 글**

이전에 다뤘던 펌웨어 추출 과정을 통해 실제 기기의 플래시메모리에서 추출한 펌웨어를 기반으로 이번 글에서는 정적분석에 대해 다룬다. 특히 MIPS 기반 임베디드 장치(Jooan C9TS IP Camera)의 부팅 과정을 정적 분석을 통해 추론한 내용을 정리한다. 추출한 펌웨어에서 커널 이미지, 루트 파일 시스템, 설정 파일, 초기화 스크립트 등을 기반으로 실제 부팅 흐름을 단계별로 재구성하였다. 이를 통해 기기의 동작 구조를 이해하고, 이후 동적 분석을 위한 기반을 마련한다.

본 문서의 구성은 다음과 같다. 1. Docker 환경에서 binwalk를 통한 정적 분석 환경 구축을 다루며, 2. binwalk를 통한 펌웨어 파일을 분석한다. 3.에서는 획득한 펌웨어의 루트 파일시스템을 분석하여 부팅과정을 추론하고 이를 기반으로 마지막 4.에서 최종 부팅 흐름을 정리한다.

### **✅ 준비사항**

- [ ]  펌웨어 확보가 되어있는가?
- [ ]  펌웨어를 제대로 추출했는지 검증했는가?

# 3.1 정적 분석 환경 구축

정적 분석은 펌웨어를 실제로 실행하지 않고도 내부 구조와 파일을 분석하는 과정이다. 이를 통해 파일 시스템, 커널 이미지, 실행 스크립트 등 기기의 동작 원리를 파악할 수 있다. 

펌웨어 파일은 바이너리 파일로 되어있어, 사람이 직접 분석하기 어렵다. 이때 Binwalk나 FMK(Firmware Mod Kit) 같은 도구를 사용하면 펌웨어 내부 구조를 자동으로 분석하고, 포함된 파일들을 추출할 수 있다. 본 실습에서 사용할 binwalk는 펌웨어 이미지에서 파일 시그니처를 기반으로 각 오프셋에 저장된 데이터를 식별해준다. 

본 실습에서는 binwalk를 사용해 정적 분석 환경을 구축하였다. 도커(Docker) 환경에서 binwalk v3.1.0를 설치하였으며, 설치 방법은 아래 github  문서를 참고하면 된다.

> https://github.com/ReFirmLabs/binwalk/wiki/Building-A-Binwalk-Docker-Image
> 

도커 외의 일반적인 환경에서도 빌드할 수 있는 방법이 안내되어 있으니 참고하면 된다.

# 3.2 정적 분석 진행

부팅 과정을  분석하면 커널 이미지, 루트 파일 시스템 마운트 경로 등을 확보해 QEMU 에뮬레이션 환경을 구축하는데 필요한 정보를 얻을 수 있다.

## 3.2.1 펌웨어 구조 분석

아래의 내용은 분석 대상 기기(Jooan c9ts IP camera)의 부팅과정을 분석한 것이다.

직접 추출한 펌웨어 파일을 대상으로 binwalk 분석을 수행한 결과, 다음과 같은 구성 요소들이 존재함을 확인하였다. 분석에는 도커 기반 `binwalkv3` 이미지를 활용하였으며, 자동 추출 옵션을 통해 파일 시스템을 분석하였다.

```bash
 sudo docker run -t -v "$PWD":/analysis binwalkv3 -Me {펌웨어파일}.bin
```

- `-M` : 추출된 파일을 재귀적으로 재스캔한다.
- `-e` : 탐지된 각 구성 요소를 자동으로 추출한다.

분석 결과, 추출한 펌웨어 파일(flash.bin) 내부에는 다음과 같은 구성 요소들이 포함되어 있었다.

![image.png](3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%20%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%2022f1e282ea158061ba1ae4beae0d0c43/image.png)

| Offset (HEX) | 구성 요소 | 설명 |
| --- | --- | --- |
| 0x48000 | uImage | LZMA 압축 커널 (Linux 3.10.14, MIPS) |
| 0x1B8000 | SquashFS 1 | 루트 파일 시스템 1 (xz 압축) |
| 0x488000 | SquashFS 2 | 루트 파일 시스템 2 (사용자 실행 스크립트 포함) |
| 0x798000 | JFFS2 | 설정 관련 파일 시스템 |

각 파일 시스템은 `extractions/flash.bin.extracted` 경로에 자동 저장되며, 해당 디렉토리를 통해 내부 구조, 실행 스크립트, 설정 파일, 바이너리 등을 확인하고 분석할 수 있다.
이 과정에서 커널 이미지 버전과 아키텍처 정보를 확보해야 한다. 이 바탕으로 추후 QEMU 실행 시 아키텍처 선택 (qemu-system-mipsel) 및 부팅 옵션을 설정할 수 있다. 또한, 커널 버전 정보를 참고하여 buildroot로 동일 버전(Linux-3.10.14)의 커널을 빌드할 수 있다.

## 3.2.2 파일 시스템 분석

### 3.2.2.1 초기화 관련

추출한 펌웨어 파일을 binwalk로 분석한 결과, 오프셋 `0x1B8000`에서 SquashFS 파일 시스템을 확인할 수 있다. 이 영역을 추출해 `squashfs-root` 디렉토리를 확보한 후, 내부에 `/bin`, `/sbin`, `/etc`, `/lib` 등 루트 파일 시스템의 핵심 디렉토리 구조가 포함되어 다. 이를 통해 해당 영역이 실제 장치에서 부팅 시 마운트되는 루트 파일 시스템(rootfs)임을 추정할 수 있다. 이후, 이 루트 파일 시스템 구조를 기반으로 실제 장치의 부팅 과정을 추적하면 된다.

![스크린샷 2025-07-13 010037.png](3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%20%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%2022f1e282ea158061ba1ae4beae0d0c43/%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7_2025-07-13_010037.png)

커널 부팅 이후 실행되는 최초의 사용자 공간 프로세스는 `/linuxrc`로 `bin/busybox`로 심볼릭 링크되어 있는 것을 통해 해당 시스템이 BusyBox 기반의 임베디드 리눅스 환경이라는 것을 알 수 있다. BusyBox는 여러 유닉스 명령어를 하나의 실행파일로 통합한 소프트웨어다. 이는 적은 용량으로 다양한 유틸리티 기능을 제공하기 때문에 공간제약이 있는 임베디드 시스템에서 유용하다.

```bash
(base) syj@syjeong:~/WHS/new$ file extractions/flash3.bin.extracted/1B8000/squashfs-root/linuxrc
extractions/flash3.bin.extracted/1B8000/squashfs-root/linuxrc: symbolic link to bin/busybox
```

`/linuxrc`가 실행되면서 BusyBox init이 시작되고, Busybox init은 기본적으로 `/etc/inittab` 파일을 읽으므로  busybox 설정은 `inittab` 파일에서 확인할 수 있다. 

![image (4).png](3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%20%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%2022f1e282ea158061ba1ae4beae0d0c43/image_(4).png)

`inittab` 파일에서 초기화 마운트 명령과 함께 rcS 스크립트를 호출하는 부분이 있다. 이는 `/etc/init.d/rcS`가 부팅 시 호출되도록 설정된 것이다.

### 3.2.2.2 설정 파일

오프셋 `0x798000`에서 설정 파일(`config.json`)을 확인할 수 있다. 해당 설정 파일은 `/opt/conf/config.json` 경로에 위치하며, 부팅 과정에서 여러 스크립트가 이 파일을 읽어 시스템 동작 방식을 결정한다.

이 파일에는 클라우드 인증키, 모바일 앱과 연결되는 P2P ID 및 계정 정보, 기기 시리얼 넘버 등이 포함되어 있다. 따라서 원격 접속, 클라우드 연동, 펌웨어 업데이트와 같은 동작에 직접적인 영향을 준다. 

![클라우드 인증 키, P2p ID 및 계정 정보, 시스템 정보 등](3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%20%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%2022f1e282ea158061ba1ae4beae0d0c43/image%201.png)

클라우드 인증 키, P2p ID 및 계정 정보, 시스템 정보 등

## 3.2.3 초기화 스크립트 분석

rcS 파일의 내부 동작은 다음과 같다.

1. **장치 노드 및 환경 설정**
    
    ```bash
    echo /sbin/mdev > /proc/sys/kernel/hotplug
    /sbin/mdev -s && echo "mdev is ok......"
    
    export PATH=/bin:/sbin:/usr/bin:/usr/sbin
    export PATH=/mnt/mtd/run:$PATH
    export LD_LIBRARY_PATH=/mnt/mtd/lib
    ```
    
    `mdev`는 리눅스에서 장치를 자동으로 인식하고 `/dev` 경로에 장치 파일을 생성하는 역할을 한다. 이를 통해 USB, 메모리 등 하드웨어 장치가 연결되었을 때 자동으로 사용 가능하도록 준비할 수 있다. 또한 `PATH`와 `LD_LIBRARY_PATH`는 프로그램이 실행 파일과 라이브러리를 찾는 경로를 지정하는 환경 변수이다. `PATH`는 실행 명령어를 탐색할 디렉토리를, `LD_LIBRARY_PATH`는 동적 라이브러리(.so)의 위치를 지정하여 프로그램이 정상적으로 동작할 수 있도록 한다.
    
2. **네트워크 설정**
    
    ```bash
    ifconfig lo up
    ifconfig eth0 172.16.1.18 netmask 255.255.255.0 up
    
    mkdir -p /var/lib/misc
    touch /var/lib/misc/udhcpd.leases
    ```
    
    네트워크 설정 과정에서는 먼저 `lo`(loopback) 인터페이스를 활성화하여 장치 내부에서 자기 자신과 통신할 수 있도록 한다. 이어서 `eth0` 인터페이스에 IP 주소(`172.16.1.18`)와 서브넷 마스크를 설정하여 유선 네트워크 연결이 가능하도록 한다. 또한 `mkdir -p /var/lib/misc` 명령으로 네트워크 서비스에서 필요한 임시 디렉터리를 생성하고, `touch /var/lib/misc/udhcpd.leases`로 DHCP 서버가 네트워크에 연결된 장치에 임시로 할당한 IP 주소 정보를 저장할 파일을 만들어 DHCP 및 HostAPD와 같은 네트워크 관리 서비스가 정상적으로 동작할 수 있도록 준비한다.
    
3. **파일 시스템 마운트**
    
    ```bash
    mount -t squashfs /dev/mtdblock4 /mnt/mtd
    mount -t jffs2 /dev/mtdblock5 /opt
    ```
    
    `/dev/mtdblock4` 장치에 저장된 squashfs 파일 시스템을 `/mnt/mtd` 경로로 마운트하여, 읽기 전용으로 제공되는 기본 시스템 파일에 접근할 수 있도록 한다. 
    `/dev/mtdblock5` 장치에 저장된 jffs2 파일 시스템을 `/opt` 경로로 마운트하여, 부팅 후 변경 가능한 설정 파일이나 로그 등 읽기·쓰기 작업이 필요한 데이터를 저장할 수 있도록 한다.
    
    이렇게 두 개의 파티션을 각각 목적에 맞게 마운트함으로써 시스템이 부팅 후 안정적으로 동작할 수 있는 파일 환경을 준비한다.
    
4. **`config.json` 구성 확인**
    
    ```bash
    if [ ! -f /opt/conf/config.json ];then
      ...
      cp /opt/conf/config.org /opt/conf/config.json
    fi
    ```
    
    `config.json` 파일이 없을 경우에, `config.org`로부터 복사해서 복구한다.
    
5. **WiFi 설정 및 드라이버 삽입 (GPIO 제어 포함)**
    
    **Wi-Fi Reset GPIO**
    
    ```bash
    wifi_ResetGpio=...
    if [ "$wifi_ResetGpio" -gt 0 ]; then
      ...
    fi
    ```
    
    `config.json`에서 Wi-Fi 리셋용 GPIO를 읽고 직접 GPIO 제어한다.
    
    **Wi-Fi Power GPIO**
    
    ```bash
    wifi_PowerGpio=...
    if [ "$wifi_PowerGpio" -gt 0 ]; then
      ...
    fi
    ```
    
    전원 관련 GPIO를 읽어서 제어한다.
    
6. **드라이버 자동 선택 및 모듈 로딩**
    
    ```bash
    if echo "$SdioID" | grep -qwi "$SDIOID_SSV6255"; then
                    echo "find sdio ssv6255 OK"
                    insmod ssv6255.ko stacfgpath=/etc/ssv6255-wifi.cfg
            elif echo "$SdioID" | grep -qwi "$SDIOID_SSV6158"; then
                    echo "find sdio ssv6158 OK"
                    insmod ssv6158.ko stacfgpath=/etc/ssv6158-wifi.cfg
    											...
    elif echo "$UsbID" | grep -qwi "$USBID_ATBM6132BU"; then
                    echo "find usb atbm6132BU OK"
                    insmod atbm6132_wifi_usb.ko wifi_bt_comb=1 #1 是 WiFi+BLE驱动 0 是完整版驱动
    ```
    
    장치에 연결된 무선 네트워크 칩의 종류(SDIO ID 또는 USB ID)를 확인하여, 이에 맞는 드라이버 모듈(`.ko` 파일)을 선택하여 커널에 로드한다. 예를 들어, SSV6255 칩이 감지되면 `ssv6255.ko` 모듈을, SSV6158 칩이 감지되면 `ssv6158.ko` 모듈을 삽입해 무선 네트워크 기능을 활성화한다.
    
    ```bash
    json_debug_rootfs -i -c w -k /FunctionList/whether_4G_wifi_device -v 1 /tmp/keyinfo.json
    ...
    json_debug_rootfs -i -c w -k /FunctionList/whether_4G_wifi_device -v 1 /opt/conf/config.json
    json_debug_rootfs -i -c w -k /FunctionList/whether_4G_wifi_device -v 1 /opt/conf/config.org
    ```
    
    `json_debug_rootfs` 명령을 통해 `config.json` 및 `keyinfo.json` 파일의 특정 키(`/FunctionList/whether_4G_wifi_device`) 값을 동적으로 수정하여, 새로운 드라이버 로딩이나 하드웨어 기능 변화를 시스템 설정에 즉시 반영한다.
    
    즉, 장치가 어떤 Wi‑Fi 칩을 사용하는지 부팅 시점에 자동으로 감지하고, 그에 맞는 드라이버와 설정을 적용함으로써 네트워크 기능이 올바르게 동작하도록 준비하는 단계이다.
    
7. **시스템 데몬 및 앱 실행**
    
    ```bash
    goahead &
    system_call_daemon &
    
    if [ -f /mnt/mtd/startapp ]; then
      /mnt/mtd/startapp &
    fi
    ```
    
    - `goahead`: 경량 웹 서버를 실행하여 장치의 웹 기반 설정 UI와 API 서비스를 제공한다.
    - `system_call_daemon`: 시스템 호출을 관리하는 백그라운드 데몬으로, 장치 전반의 서비스 요청을 처리한다.
    - `startapp`: `/mnt/mtd/startapp` 파일이 존재할 경우 실행되며, 카메라 장치의 센서 제어나 영상 처리 등의 애플리케이션을 시작한다.
    
    모두 백그라운드로 실행(`&`)되기 때문에 부팅 후에도 장치가 사용자 요청에 응답하면서 지속적으로 동작할 수 있다.
    

rcS 스크립트는 시스템의 핵심 구성 요소들을 초기화하고, 사용자 애플리케이션을 실행하는 중심 역할을 수행함을 확인할 수 있다.

## 3.2.4 실행 스크립트 분석

`jzstart.sh`: 하드웨어 드라이버 초기화

이 스크립트는 `config.json`의 설정 값을 기반으로 다음 커널 모듈들을 동적으로 로딩한다.

| 구성 요소 | 커널 모듈 (.ko) | 설명 |
| --- | --- | --- |
| ISP | `tx-isp.ko` | 이미지 처리 장치 초기화 |
| LCD | `lcd-device.ko`, `jzfb.ko` | 화면 드라이버 |
| 센서 | `sensor_xxx.ko` | 실제 연결된 센서 모델을 자동으로 감지한 후, 해당 드라이버 모듈을 로딩함 |
| PTZ | `sample_motor.ko` | 모터 회전 및 줌 컨트롤 |
| 오디오 | `audio.ko` | 볼륨, 마이크, 스피커 설정 |
| 4G/USB | `dwc2.ko`, `usbserial.ko` 등 | 외부 통신 장치 연결 |

## 3.2.5 실행되는 주요 프로그램

부팅이 끝나면 장치가 정상 동작하기 위해 여러 프로그램이 자동으로 실행된다. 이 중 어떤 프로그램을 먼저 살펴볼지는 ‘서비스에 직접적인 영향을 주는가’가 중요한 기준이 된다.

기기에서 실행되는 주요 프로그램은 다음과 같다:

| 실행 파일 | 역할 |
| --- | --- |
| `goahead` | 경량 웹서버 (장치 설정 UI/API 제공) |
| `jooanipc` | IPC(Inter-Process Communication) 애플리케이션 |
| `my_watch_dog` | 시스템 watchdog 데몬 |
| `syslogd` | 로깅 데몬 |
| `kernelinfo` | 커널 정보 수집 |
- `goahead`
외부에서 접근 가능한 웹 인터페이스를 제공하므로, 취약점이 발견될 경우 원격 공격에 악용될 수 있다.
- `jooanipc`
카메라 핵심 동작을 담당하는 프로세스이므로, 디버깅 시 가장 많은 정보를 얻을 수 있는 대상이라 추측한다.

즉, 서비스 동작과 보안 모두에 영향을 크게 미치는 초기 프로세스이기 때문에 가장 먼저 분석 대상으로 삼았다.

# 3.3 최종 부팅 흐름

![image.png](3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%20%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%2022f1e282ea158061ba1ae4beae0d0c43/image%202.png)

# **✅** 완료 판단 기준

- [ ]  파일 시스템 마운트 위치를 확인했는가?
- [ ]  커널 이미지 버전을 확인했는가?
- [ ]  기기의 사용 아키텍처를 확인했는가?
- [ ]  동적 분석할 대상을 정했는가?