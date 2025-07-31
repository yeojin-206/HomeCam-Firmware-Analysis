FROM ubuntu:16.04

#     빌드에 필요한 모든 필수 도구들을 미리 설치
#    (apt-get이 non-interactive 모드로 실행되도록 설정)
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    libncurses5-dev \
    bc \
    unzip \
    cpio \
    fakeroot \
    cmake \
    m4 \
    wget \
    sudo \
    libssl-dev \
    --no-install-recommends

#  sudo 권한을 가진 빌드용 사용자 생성
RUN useradd -m -s /bin/bash builduser && \
    echo "builduser:builduser" | chpasswd && \
    adduser builduser sudo

#  사용자를 builduser로 전환
USER builduser
WORKDIR /home/builduser

# 이 컨테이너가 실행될 때 기본으로 실행할 명령어
CMD ["/bin/bash"]
