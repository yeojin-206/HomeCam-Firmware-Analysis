### **시작하는 글**

앞선 펌웨어 추출 과정을 통해 확보한 실제 기기(Jooan C9TS IP 카메라)의 펌웨어를 기반으로, 본 문서에서는 대상 기기의 아키텍처를 확인하고 부팅 과정을 추론하는 정적 분석 절차를 다룬다.

먼저, Docker 환경에서 Binwalk 설치 및 기본 사용 방법을 설명한 후, 이를 활용해 펌웨어에서 커널 이미지, 루트 파일 시스템, 설정 파일, 초기화 스크립트 등을 추출한다. 이후 각 구성 요소를 정적으로 분석하여 부팅 흐름을 단계별로 재구성하고, 기기의 동작 구조를 파악함으로써 향후 동적 분석을 위한 기반을 마련한다.

### **✅ 준비사항**

- [ ]  펌웨어 추출 과정이 정상적으로 진행되었는가?
- [ ]  반복 추출 과정을 통해 펌웨어의 일관성을 확인하였는가?

# 3.1 정적 분석 환경 구축

정적 분석은 펌웨어를 실행하지 않고 내부 구조와 파일을 검토해 기기의 동작 원리를 파악하는 과정이다. 이 과정에서 파일 시스템, 커널 이미지, 실행 스크립트 등을 확인한다.

펌웨어 이미지는 바이너리 형태로 제공되어 사람이 직접 분석하기 어렵다. 이때 Binwalk나 FMK(Firmware Mod Kit) 같은 도구를 사용하면 펌웨어 내부 구조를 자동으로 분석하고, 포함된 파일들을 추출할 수 있다. 본 실습에서는 Binwalk를 활용하며, 이 도구는 파일 시그니처를 기반으로 이미지 내 각 오프셋의 데이터를 식별·추출한다.

정적 분석 실습 환경은 Docker 컨테이너에 Binwalk v3.1.0을 설치하여 구성하였다. 자세한 설치 방법은 아래 GitHub 문서에서 확인할 수 있다. 

> https://github.com/ReFirmLabs/binwalk/wiki/Building-A-Binwalk-Docker-Image
> 

# 3.2 정적 분석 진행

본 절에서는 Binwalk를 활용해 대상 기기의 펌웨어에서 파일을 추출하고, 정적 분석을 통해 커널 버전, 루트 파일 시스템 마운트 경로, 부팅 과정 등 QEMU 에뮬레이션 환경 구축에 필요한 정보를 확보한 과정을 설명한다.

## 3.2.1 펌웨어 구조 분석

앞서 준비한 Binwalk Docker 컨테이너를 활용하여, 대상 기기에서 추출한 펌웨어 이미지에 자동 추출 옵션을 적용하고 펌웨어 내부 구조를 확인하였다. 

```bash
 sudo docker run -t -v "$PWD":/analysis binwalkv3 -Me {펌웨어 파일}
```

- `-M` : 추출된 파일을 재귀적으로 재스캔한다.
- `-e` : 탐지된 각 구성 요소를 자동으로 추출한다.

Binwalk 분석 결과, 추출한 펌웨어 이미지 내부에선 다음과 같은 구성 요소들이 확인되었다. 

![image.png](attachment:e68420f9-8297-4de7-96ec-67067bc146e8:image.png)

| Offset(HEX) | 구성 요소 | 설명 |
| --- | --- | --- |
| `0x48000` | uImage | LZMA 압축 커널 (Linux 3.10.14, MIPS) |
| `0x1B8000` | SquashFS 1 | 루트 파일 시스템 1 (xz 압축) |
| `0x488000` | SquashFS 2 | 루트 파일 시스템 2 (사용자 실행 스크립트 포함) |
| `0x798000` | JFFS2 | 설정 관련 파일 시스템 |

추출된 파일 시스템은 `extractions/flash.bin.extracted` 경로에 자동으로 저장되며, 해당 디렉터리에서 내부 구조, 실행 스크립트, 설정 파일, 바이너리 등을 확인하고 분석할 수 있다. 

Binwalk의 Description 항목에서 대상 기기가 MIPS 32‑bit 아키텍처이며 Linux 커널 3.10.14 버전을 사용중임을 확인했다. 또한 파일 시스템들이 공통적으로 Little Endian 방식을 사용하고 있는 것을 통해 기기의 엔디언 설정이 Little Endian임이 확인되었다.

이렇게 확인된 커널 버전 및 아키텍처 정보는 추후 동적 분석 환경 구축을 위한 커널 이미지 준비와 QEMU 설정에 활용된다.

## 3.2.2 파일 시스템 분석

### 3.2.2.1 초기화 관련

Binwalk 분석 결과, 오프셋 `0x1B8000`에서 SquashFS 파일 시스템이 확인되었다. 해당 영역을 추출해 생성된 `squashfs-root` 디렉터리를 확인한 결과, `/bin`, `/sbin`, `/etc`, `/lib` 등 루트 파일 시스템의 핵심 디렉터리 구조를 발견할 수 있었다. 이를 통해 해당 영역이 부팅 시 장치에서 마운트되는 루트 파일 시스템(RootFS)임을 확인하였으며, 이후 이 루트 파일 시스템을 기반으로 실제 장치의 부팅 과정을 단계별로 추적하였다.

![스크린샷 2025-07-13 010037.png](attachment:37a8f635-faa7-49b4-b680-599e5f29b827:스크린샷_2025-07-13_010037.png)

초기 실행 파일을 식별하기 위해 펌웨어에서 문자열을 검색하여 U-Boot 부팅 인자를 확인하였다. 사용한 명령어는 다음과 같다. 

```bash
strings {펌웨어 파일} | grep -i "init"
```

- `strings`: 바이너리 파일에서 문자열을 찾아 텍스트로 출력한다.
- `grep -i “init”`: 문자열의 대소문자를 구분하지 않고 “init”이 포함된 모든 줄을 출력한다.

이 명령을 추출한 펌웨어 이미지에 적용하여 다음과 같은 `bootargs` 문자열을 확인하였다.

![image.png](attachment:d555088b-f7a0-47ed-a9a0-fff3a372f03a:image.png)

`bootargs`에 `init=/linuxrc`가 설정되어 있음을 확인할 수 있다. 이는 커널 부팅 직후 사용자 공간에서 실행할 첫 프로세스를 `/linuxrc`로 지정한다는 것을 의미한다. `/linuxrc`는 루트 파일 시스템을 마운트하고 필수 드라이버를 로드하는 등 본격적인 `init` 실행 이전에 초기 환경을 구성하는 역할을 수행한다.

![image.png](attachment:963599ca-7459-4d37-a433-99e05cc1bc4a:image.png)

파일의 형식을 확인할 수 있는 `file` 명령어를 이용해 `linuxrc` 파일을 확인하면 `linuxrc`가 `/bin/busybox`로 연결된 심볼릭 링크임을 확인할 수 있다. 이 결과를 통해, 시스템이 BusyBox 기반의 임베디드 리눅스 환경으로 구성되었음을 확인할 수 있다.

**BusyBox란?**

BusyBox는 여러 유닉스 명령어를 하나의 실행 파일로 통합한 소프트웨어다. 이는 적은 용량으로 다양한 유틸리티 기능을 제공하기 때문에 공간 제약이 있는 임베디드 시스템에서 유용하다.

`/linuxrc`가 실행되면 BusyBox의 `init` 프로세스가 시작되고, 이 `init`은 `/etc/inittab`을 읽어 초기 서비스 설정을 수행한다.

![image (4).png](attachment:d36912b7-6cd8-4518-91c9-4e8659a42ce8:image_(4).png)

`inittab` 파일에는 초기화 마운트 명령과 함께 `rcS` 스크립트를 호출하는 항목이 포함되어 있다. 이를 통해 부팅 시 `rcS` 스크립트가 자동으로 실행됨을 알 수 있다

### 3.2.2.2 설정 파일

펌웨어의 JFFS2 파일 시스템(오프셋 `0x798000`) 내부에선 `/opt/conf/config.json` 경로의 설정 파일을 확인할 수 있다. 해당 파일에는 클라우드 인증 키, 모바일 앱 연동용 P2P ID와 계정 정보, 기기 시리얼 번호 등이 포함되어 있다. 이 설정 값들은 원격 접속·클라우드 연동·펌웨어 업데이트 등 주요 기능에 활용되며, 부팅 과정에서 여러 초기화 스크립트가 이 파일을 참조하여 동작한다.

![클라우드 인증 키, P2p ID 및 계정 정보, 시스템 정보 등](attachment:f6c99fb1-9d60-4482-ab4b-9a67d9f78bce:image.png)

클라우드 인증 키, P2p ID 및 계정 정보, 시스템 정보 등

## 3.2.3 초기화 스크립트 분석

다음은 `rcS` 초기화 스크립트의 주요 동작 과정이다.

1. **장치 노드 및 환경 설정**
    
    ```bash
    echo /sbin/mdev > /proc/sys/kernel/hotplug
    /sbin/mdev -s && echo "mdev is ok......"
    
    export PATH=/bin:/sbin:/usr/bin:/usr/sbin
    export PATH=/mnt/mtd/run:$PATH
    export LD_LIBRARY_PATH=/mnt/mtd/lib
    ```
    
    `mdev`는 리눅스에서 장치를 자동으로 인식하고 `/dev` 경로에 장치 파일을 생성하는 역할을 한다. 이를 통해 USB, 메모리 등 하드웨어 장치가 연결되었을 때 자동으로 사용 가능하도록 준비한다. 
    
    `PATH`와 `LD_LIBRARY_PATH`는 프로그램이 실행 파일과 라이브러리를 찾는 경로를 지정하는 환경 변수이다. `PATH`는 실행 명령어를 탐색할 디렉터리를, `LD_LIBRARY_PATH`는 동적 라이브러리(`.so`)의 위치를 지정하여 프로그램이 정상적으로 동작할 수 있도록 한다. 
    
2. **네트워크 설정**
    
    ```bash
    ifconfig lo up
    ifconfig eth0 172.16.1.18 netmask 255.255.255.0 up
    
    mkdir -p /var/lib/misc
    touch /var/lib/misc/udhcpd.leases
    ```
    
    `lo`(loopback) 인터페이스를 활성화하여 장치 내부 통신을 가능하게 한다. 이어서 `eth0` 인터페이스에 IP 주소(`172.16.1.18`)와 서브넷 마스크를 할당하여 유선 네트워크 연결을 설정한다. 
    
    `mkdir -p /var/lib/misc` 명령으로 네트워크 서비스에서 필요한 임시 디렉터리를 생성하고, `touch /var/lib/misc/udhcpd.leases`로 DHCP 서버가 네트워크에 연결된 장치에 임시로 할당한 IP 주소 정보를 저장할 파일을 만들어 DHCP 및 HostAPD와 같은 네트워크 관리 서비스가 정상적으로 동작할 수 있도록 준비한다.
    
3. **파일 시스템 마운트**
    
    ```bash
    mount -t squashfs /dev/mtdblock4 /mnt/mtd
    mount -t jffs2 /dev/mtdblock5 /opt
    ```
    
    `/dev/mtdblock4`에 저장된 SquashFS 파일 시스템을 `/mnt/mtd`에 마운트하여, 읽기 전용 기본 시스템 파일에 접근할 수 있도록 한다. 
    `/dev/mtdblock5` 장치에 저장된 JFFS2 파일 시스템을 `/opt`에 마운트하여, 부팅 후 변경될 수 있는 설정 파일이나 로그 등 읽기·쓰기 작업이 필요한 데이터를 저장할 수 있도록 한다.
    
    이처럼 두 파티션을 각각의 목적에 맞게 마운트하여 시스템이 부팅 후 안정적으로 동작할 수 있는 파일 환경을 구성한다.
    
4. **`config.json` 구성 확인**
    
    ```bash
    if [ ! -f /opt/conf/config.json ];then
      ...
      cp /opt/conf/config.org /opt/conf/config.json
    fi
    ```
    
    `config.json` 파일이 없을 경우, `config.org`로부터 복사해서 복구한다.
    
5. **WiFi 설정 및 드라이버 삽입(GPIO 제어 포함)**
    
    **Wi-Fi Reset GPIO**
    
    ```bash
    wifi_ResetGpio=...
    if [ "$wifi_ResetGpio" -gt 0 ]; then
      ...
    fi
    ```
    
    `config.json`에서 Wi-Fi 리셋용 GPIO 번호를 읽고 해당 GPIO를 제어한다.
    
    **Wi-Fi Power GPIO**
    
    ```bash
    wifi_PowerGpio=...
    if [ "$wifi_PowerGpio" -gt 0 ]; then
      ...
    fi
    ```
    
    `config.json`에서 전원 관련 GPIO 번호를 읽고 해당 GPIO를 제어한다.
    
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
    
    `json_debug_rootfs` 명령을 통해 `config.json` 및 `keyinfo.json` 파일의 특정 키(`/FunctionList/whether_4G_wifi_device`) 값을 런타임에 동적으로 수정함으로써, 부팅 중에 필요한 새로운 드라이버를 로드하나 하드웨어 설정 변화를 시스템 설정에 즉시 반영할 수 있다.
    
    즉, 부팅 시점에 장치가 사용하는 Wi‑Fi 칩을 자동으로 감지하고, 그에 맞는 드라이버와 설정을 적용함으로써 네트워크 기능이 올바르게 동작하도록 준비하는 단계이다.
    
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
    - `startapp`: `/mnt/mtd/startapp` 파일이 존재할 경우 실행되며, 카메라 장치의 센서 제어나 영상 처리 등의 애플리케이션을 실행한다.
    
    모두 백그라운드에서 실행(`&`)되기 때문에 부팅 후에도 장치가 사용자 요청에 응답하면서 지속적으로 동작할 수 있다.
    

이처럼 `rcS`스크립트는 시스템 핵심 구성 요소를 초기화하고, 사용자 애플리케이션 실행을 위한 환경을 구축하는 역할을 수행한다. 

## 3.2.4 실행 스크립트 분석

`jzstart.sh`는 하드웨어 드라이버 초기화를 담당하는 스크립트이다. 

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

부팅이 완료되면 장치의 정상 동작을 위해 여러 프로그램이 자동으로 실행된다. 정적 분석을 통해 서비스에 직접적인 영향을 미치는 프로그램을 파악하고, 이를 기준으로 동적 분석 대상을 선정한다.

기기에서 실행되는 주요 프로그램은 다음과 같다. 

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
jooanipc는 goahead의 CGI 요청을 처리하며 웹 서버 기능을 확장한다. 이로 인해 취약점이 발견될 경우 goahead와 마찬가지로 원격 공격에 악용될 수 있다.

이들 프로그램은 서비스의 핵심 기능을 담당하고 임베디드 기기의 보안에 직접적인 영향을 미칠 수 있으므로, 이후 동적 분석 대상으로 선정했다.

# 3.3 최종 부팅 흐름

![image.png](attachment:06326841-e9bb-43f0-b58e-244c929a6073:image.png)

# **✅** 완료 판단 기준

- [ ]  기기에 사용된 아키텍처를 확인했는가?
- [ ]  펌웨어에 사용된 커널 이미지 버전을 확인했는가?
- [ ]  파일 시스템이 마운트되는 위치를 확인했는가?
- [ ]  동적 분석 대상을 선정했는가?
