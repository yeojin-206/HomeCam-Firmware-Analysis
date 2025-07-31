# Firmware Analysis Framework for Embedded


**화이트햇스쿨 작고소중해 팀 프로젝트: 소형 임베디드 기기 펌웨어 분석 & QEMU 기반 동적 분석 환경 구축**

> 프로젝트 기간 : 2025년 5월 ~ 2025년 8월 (약 3개월)
> 

본 프로젝트는 소형 임베디드 기기에서 펌웨어를 추출·분석하고, QEMU 기반의 동적 분석 환경을 구축하는 과정을 체계적으로 학습하는 데 목적을 두고 진행되었다. 해당 과정을 문서화하여 정리함으로써 추후 펌웨어 분석의 가이드라인으로 활용할 수 있는 재현성 높은 워크플로우를 제공하고자 한다.

## 저자


> **화이트햇 스쿨 3기 작고소중해 팀**
> 

Mentor: 문석주

PL(Project Leader): 장형범

김여진(PM), 강주영, 이관호, 이동환, 이준희, 정서윤, 최정민, 최진범

## 카테고리

### 1. [사전지식](1%20사전지식/README.md)

- [임베디드 시스템 기초](1%20사전지식/1%20임베디드%20시스템%20기초/README.md)  
- [펌웨어 추출 도구](1%20사전지식/2%20펌웨어%20추출%20도구/README.md)  
- [펌웨어 분석 도구](1%20사전지식/3%20펌웨어%20분석%20도구/README.md)


### 2. [펌웨어 추출](2%20펌웨어%20추출/README.md)

- [펌웨어 추출 개요](2%20펌웨어%20추출/README.md)
- [오픈소스 펌웨어 활용](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/2%20%E1%84%91%E1%85%A5%E1%86%B7%E1%84%8B%E1%85%B0%E1%84%8B%E1%85%A5%20%E1%84%8E%E1%85%AE%E1%84%8E%E1%85%AE%E1%86%AF#1-%EC%98%A4%ED%94%88%EC%86%8C%EC%8A%A4-%ED%8E%8C%EC%9B%A8%EC%96%B4-%ED%99%9C%EC%9A%A9)
- [SPI 통신 활용](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/2%20%E1%84%91%E1%85%A5%E1%86%B7%E1%84%8B%E1%85%B0%E1%84%8B%E1%85%A5%20%E1%84%8E%E1%85%AE%E1%84%8E%E1%85%AE%E1%86%AF#2-spi-%ED%86%B5%EC%8B%A0%EC%9D%84-%ED%99%9C%EC%9A%A9%ED%95%9C-%ED%8E%8C%EC%9B%A8%EC%96%B4-%EC%B6%94%EC%B6%9C)

### 3. [정적 분석](3%20정적분석/README.md)

- [정적 분석 개요](3%20정적분석/README.md)
- [정적 분석 환경 구축](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8#31-%EC%A0%95%EC%A0%81-%EB%B6%84%EC%84%9D-%ED%99%98%EA%B2%BD-%EA%B5%AC%EC%B6%95)
- [정적 분석 진행](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8#32-%EC%A0%95%EC%A0%81-%EB%B6%84%EC%84%9D-%EC%A7%84%ED%96%89)
- [최종 부팅 흐름](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/3%20%E1%84%8C%E1%85%A5%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8#33-%EC%B5%9C%EC%A2%85-%EB%B6%80%ED%8C%85-%ED%9D%90%EB%A6%84)

### 4. [동적 분석을 위한 환경 구축](4%20동적분석을%20위한%20환경구축/README.md)

- [동적 분석 개요](4%20동적분석을%20위한%20환경구축/README.md)
- [커널 이미지 생성](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/4%20%E1%84%83%E1%85%A9%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%E1%84%8B%E1%85%B3%E1%86%AF%20%E1%84%8B%E1%85%B1%E1%84%92%E1%85%A1%E1%86%AB%20%E1%84%92%E1%85%AA%E1%86%AB%E1%84%80%E1%85%A7%E1%86%BC%E1%84%80%E1%85%AE%E1%84%8E%E1%85%AE%E1%86%A8#41-builldroot%EB%A5%BC-%ED%86%B5%ED%95%9C-linux-31014-%EC%BB%A4%EB%84%90%EC%9D%B4%EB%AF%B8%EC%A7%80-%EC%83%9D%EC%84%B1)
- [파일 시스템 구성](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/4%20%E1%84%83%E1%85%A9%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%E1%84%8B%E1%85%B3%E1%86%AF%20%E1%84%8B%E1%85%B1%E1%84%92%E1%85%A1%E1%86%AB%20%E1%84%92%E1%85%AA%E1%86%AB%E1%84%80%E1%85%A7%E1%86%BC%E1%84%80%E1%85%AE%E1%84%8E%E1%85%AE%E1%86%A8#42-%ED%8C%8C%EC%9D%BC-%EC%8B%9C%EC%8A%A4%ED%85%9C-%EA%B5%AC%EC%84%B1)
- [동적 분석 진행](https://github.com/yeojin-206/HomeCam-Firmware-Analysis/tree/main/4%20%E1%84%83%E1%85%A9%E1%86%BC%E1%84%8C%E1%85%A5%E1%86%A8%E1%84%87%E1%85%AE%E1%86%AB%E1%84%89%E1%85%A5%E1%86%A8%E1%84%8B%E1%85%B3%E1%86%AF%20%E1%84%8B%E1%85%B1%E1%84%92%E1%85%A1%E1%86%AB%20%E1%84%92%E1%85%AA%E1%86%AB%E1%84%80%E1%85%A7%E1%86%BC%E1%84%80%E1%85%AE%E1%84%8E%E1%85%AE%E1%86%A8#43-qemu-%ED%99%98%EA%B2%BD%EC%97%90%EC%84%9C-gdb-%EC%97%B0%EA%B2%B0--%EC%9B%B9%EC%84%9C%EB%B2%84-%EB%B6%84%EC%84%9D)


