# TSLObjectPool
C++ library implementing an object pool using Thread Local Storage (TLS). Designed for x64 Windows platforms.

## 목차
[1. 오브젝트 풀 구조](https://github.com/YoungWoo93/TSLObjectPool/tree/main?tab=readme-ov-file#1-%EC%98%A4%EB%B8%8C%EC%A0%9D%ED%8A%B8-%ED%92%80-%EA%B5%AC%EC%A1%B0)
>  1) 전체 구조<br>
>  2) main pool 구조<br>
>  3) tls pool 구조<br>
  
[2. main pool](https://github.com/YoungWoo93/TSLObjectPool/tree/main?tab=readme-ov-file#2-main-pool)
>1) main pool 목적<br>
>2) main pool 동작<br>
>3) chunk stack<br>
>4) block collector<br>
>

[3. tls pool](https://github.com/YoungWoo93/TSLObjectPool/tree/main?tab=readme-ov-file#3-tls-pool)
>1) tls pool 목적<br>
>2) tls pool 동작<br>
>3) chunk cache<br>
>

[4. 성능 평가](https://github.com/YoungWoo93/TSLObjectPool/tree/main?tab=readme-ov-file#4-%EC%84%B1%EB%8A%A5%ED%8F%89%EA%B0%80)
>
>
>
<br>
<br>
<br>  

## 1. 오브젝트 풀 구조<br>
#### 1) 전체 구조<br>
> ![image01](https://github.com/user-attachments/assets/47e19cad-3f88-45b4-99b9-9c169ed53ebc)
>   
> 개별 스레드에서 메모리를 동적 할당 하는 것 이 아닌 메모리 할당, 반환을 전담하는 주체를 둔다.<br>
> 커널 모드로의 전환 및 스레드간 바쁜 대기를 최소화 하기 위한 방법이다.<br>
> <br>
> 추가로 이러한 방식으로 메모리 관리를 중앙 집중 하는경우 메모리 관리 주체만 확인해도 어떤 스레드에서, 어떤 유형의 메모리를 많이 할당하는지 확인이 가능하다.<br>
> 성능적 강점과 모니터링상 강점이 둘 다 있는 중요한 개념이라고 생각한다.<br>
> <br>
> 모든 사용자가 이러한 main pool과 tls pool에 모두 직접 접근하는것을 원치 않았다. <br>
> 왜냐하면 구조를 복잡하다고 생각 할 수 있었기 때문이며, 이 부분은 이 라이브러리를 사용하는 미래의 나에게도 적용되는 이야기이다. <br>
> 그렇기 때문에 기본적인 사용은 모두 TLSObjectPool 이라는 인터페이스의 역할을 하는 클래스 함수를 이용해 사용 가능하도록 구성하였다. <br>

<br>

#### 2) main pool 구조<br>
>  main pool은 템플릿 타입의 1개의 변수 메모리 (이하 블록, block)를 N개 묶은 템플릿 타입의 변수 덩어리 (이하 청크, chunk) 를 조작 단위로 사용한다.<br>  
> <br>  
> ![image02](https://github.com/user-attachments/assets/48b4d35e-4222-4f68-bb83-d02619fc6ccc)<br>  
> <br>
> chunk와 block은 다음과 위와 같은 구조로 되어있다.<br>  
> block의 key(= poolPtr =next) 변수는 해당 block의 위치와 옵션에 따라 여러가지 정보를 의미 할 수 있다.<br>
> 가장 기본적으로는 해당 블록의 다음 블록을 의미하는 next로 사용되지만, 할당이 된 블록은 돌아와야 할 pool의 주소를 의미한다. <br>
> 만약 MEMORY_CORRUPTED_CHECK 옵션이 켜져 있는경우, 비트마스킹 정보가 추가된다. <br>
> <br>
> chunk의 next 변수는 청크들을 연결하는 연결리스트에 사용된다.<br>
> blockStart / blockEnd 변수는 해당 청크가 담고있는 블록들(연결리스트로 연결된 N개의 덩어리 전체)의 시작과 끝을 담고있다.<br>
> capacity는 해당 청크에 담을 수 있는 최대 블록 갯수가 저장된다. <br>
> <br>
> <br>
> ![image03](https://github.com/user-attachments/assets/631527d3-5bfe-4998-aaa3-c8c4f6ebeab0)<br>
> <br>
> main pool 내에는 위에서 나온 청크를 보관하는 스택이 2종개 존재한다. <br>
> 이미 블록이 모두 차있는 청크를 다루는 fill chunk stack과 빈 청크를 다루는 empty chunk stack 이다. <br>
> block collector는 1개의 chunk를 다 채우지 못하는, 조각난 블록들을 처리하기 위한 객체이다. <br>
<br>

#### 3) tls pool 구조<br>
> tls pool은 각 스레드의 정적 tls공간을 이용하며, 실질적으로 개발자가 메모리를 받아오게 되는 객체이다. < br>
> <br>
> ![image04](https://github.com/user-attachments/assets/7f087b1c-3c3d-4c0b-9c03-5d0ae0d1db8d) <br>
> <br>
> tls pool 내에는 메모리 블록들을 사용하기 위한 포인터들과 메모리반환시 선형 탐색을 줄이기위한 장치들이 존재한다. <br>
> 그중 chunkCache는 큐 형태로 동작하며, 선형 탐색을 줄이기 위한 핵심 아이디어가 들어간 객체이다.<br>
> <br> 
> 
> ![image05](https://github.com/user-attachments/assets/41915ac8-58c0-4b6a-a0fd-eb536ef4ada4) <br>
> <br>
> chunkCache가 큐 형태라고는 하지만 STL 큐를 쓴다면 본 라이브러리의 의미가 조금 훼손된다고 생각하였다. <br>
> 그래서 배열을 이용한 환형 큐 형태의 구조를 구현하였다. <br>
>

<br>
<br>
<br>  

## 2. main pool<br>
#### 1) main pool의 목적<br>
> main pool의 핵심 목적은 OS의 힙 매니저의 역할을 유저영역에서 동작하게 하는 것 이다. <br>
> 그렇기 때문에 템플릿 타입별로 main pool은 1개만 존재하여야 한다.<br>
> main pool이 싱글톤 패턴으로 구현된 이유이다.<br>
> <br>
> 부가적인 목적으로 모니터링을 위한 정보를 생산 할 수 있는 주체이기도 하다.<br>
> 예를들어 empty chunk stack의 크기를 이용한다면 현재 각 tls 풀에 배포되어 사용중인 블록의 갯수를 알 수 있다.<br>
> 또는 fill chunk stack의 크기와 last used tick등을 이용한다면 한때 할당되었으나 현재는 사용되지 않는 블록도 알 수 있다.<br>
> 그래서 main pool에게 모니터링의 기능도 추가해 주기로 하였다.<br>
<br>

#### 2) main pool 동작<br>
> main pool의 동작은 간단하다.<br>
><br>
> (1) tls pool의 요청에 의해 fill chunk의 블록을 tls 풀에게 넘긴다 - 할당 동작<br>
> (2) tls pool의 요청에 의해 empty chunk의 블록을 tls 풀에게 넘긴다. (이후 가득 찬 청크를 fill chunk에 넣는다) - 반환동작<br>
> *(3) object pool manager의 요청에 의해 유휴 상태인 청크를 OS에 반환한다 - 메모리 관리 동작* .... 구현중<br>
> 
<br>

#### 3) chunk stack<br>
> <br>

> ![image06](https://github.com/user-attachments/assets/09bcc109-1a9c-46f6-8005-7a61322df090)<br>
> <br>
> 
> chunk stack은 여러 스레드에서 동시에 메모리 할당 / 반환 요청이 올 것을 대비해야한다. <br>
> 그렇기 때문에 동기화 정책을 준비해야 하고, chunk stack은 동기화 문제를 lock free 구조를 통해 해결하고자 하였다.<br>
> <br>
> 단, 기본적인 push, pop 동작은 lock free 구조로 해결을 하였지만, 메모리 관리 동작 중에는 배타적인 접근을 보장하여야 한다.<br>
> 그렇기 때문에 메모리 관리 동작을 위한 SRW lock이 존재하며, 기본적으로는 공유잠금을 이용해 사용된다.<br>
> 만약 메모리 스케일링 동작이 필요한 경우 해당 lock을 배타적으로 잠그게 되며 이를 통해 스케일링 동작때 동기화를 보장한다.<br>
> <br>
> 
>
<br>

#### 4) block collector<br>
> block collector는 1개의 청크를 충분히 채우지 못하는, 파편화된 블록을 모아 하나의 청크로 만드는 동작을 수행한다.<br>
> 
> <br>

> ![image07](https://github.com/user-attachments/assets/a2df2440-f779-41b8-beb0-0b9add62f61d)<br>
> <br>
> 
> block collector가 동작하는 상황은 크게 2가지가 있을 수 있다.<br>
> (1) 스레드가 종료되거나 tls pool이 clear 되어서 tls pool에 남아있는 잔여 블록을 일괄 반환 하는 경우. <br>
> (2) 모종의 이유로 tls 풀의 정책을 무시한채, 사용자가 강제로 메모리 반환을 하는 경우.<br>
> <br>
> 이러한 동작을 지원하기 위하여 block collector에는 항상 1개의 빈 청크가 존재하게 된다.<br>
> block collector에서 모인 블록들은 1회의 선형 탐색 후 준비된 빈 청크에 담겨 fill chunk stack으로 넘어가게 된다.<br>
><br>
> 또한 이 과정에서 각 스레드가 직접 block collector를 조작 하기 때문에 동기화 문제가 발생 가능하다.<br>
> 이 동기화 문제는 lock을 통해서 해결하고자 하였는데, 그 이유는 block collector의 동작 빈도가 그리 많지 않을 것으로 예상 되었기 때문이다.<br>
> 배타적 잠금을 하더라도 시행 횟수가 적고, 한번의 lock에서 체류하는 시간이 짧다면 시스템 전체에 가해지는 부담은 매우 적을 것이라 생각했다.<br>
> 

<br>
<br>
<br>  

## 3. tls pool<br>
#### 1) tls pool 목적<br>
> 오브젝트 풀을 사용할 때, 1개의 블록마다 메인 풀에 반복해서 접근하는 것은 많은 오버헤드를 만든다. <br>
> 그렇기 때문에 결국 청크단위로 메인 풀에서 메모리를 할당 / 반환 하고자 하였다. <br>
> <br>
> 그렇다면 청크단위로 메모리를 할당 / 반환 할때 각 스레드에서 청크 내외 크기의 메모리를 관리 해 줄 대상이 필요해진다.<br>
> 또한 해당 대상은 각 스레드마다 존재해야 하며, 각 스레드 내에서 같은 템플릿 타입에 대해서는 1개만 존재해야 한다.<br>
> 이를 극복하기 위해 스레드마다의 전역 변수라 할 수 있는 thread local storage를 이용하기로 하였다.<br>
> <br>
> tls pool은 각 스레드에서 템플릿 타입마다 1개 존재하며, 사용자가 모르게 main pool과 데이터를 교환하는 일을 수행한다.<br>
> 결국 사용자 효율적으로 오브젝트 풀을 사용 할 수 있도록 계층을 분리하는 것이 tls pool의 주 목적이라고 할 수 있다.<br>
> 
<br>

#### 2) tls pool 동작<br>
> 실제 사용자가 오브젝트 풀을 사용하데 필요한 기능들이 구현되어있다. <br>
> 하지만 사용자가 직접 tls pool의 메소드를 호출하는 것을 바라지는 않았기 때문에 다음과 같은 인터페이스를 한층 두었다. <br>
> <br>
> ![image08](https://github.com/user-attachments/assets/f4a70ca7-fcc0-48fc-929a-89d6ed9c38bd)<br>
> <br>
> 인터페이스로 노출된 메모리 할당 / 반환 동작 외에도 내부적으로 3가지 동작을 더 수행한다.<br>
> <br>
> 첫번째는 하나의 스레드에 묶여있는 메모리를 항상 일정하게 유지시키는 동작이다.<br>
> 일정 갯수 이상의 블록이 모인경우 tls pool은 main pool로 청크 단위의 반환을 시도한다. <br>
> 반대로 일정 갯수 미만의 블록만 남은경우 tls pool은 main pool에서 청크 단위 할당을 시도한다. <br>
> 이를통해 사용자는 다른 부분은 신경쓰지 않고 항상 tls pool 만을 통하여 메모리를 할당 / 반환 받을 수 있다.<br>
> <br>
> 두번째는 할당되었던 블록의 오류 체크이다.<br>
> 검출하는 오류는 2가지로 올바른 풀에 반환이 되었는지, 메모리가 오염 (오버플로우) 되지는 않았는지를 확인한다.<br>
> <br>
> 마지막은 청크단위 할당 / 반환을 위한 캐싱 동작이다.<br>
> chunk cache에서 구현된 기능을 통해, 블록을 반환할 때 1개의 청크를 채우기 위하여 선형 탐색을 하지 않는다.<br>
> chunk cache의 동작을 위해서 tls pool은 블록단위 push, pop 방향과, 청크 단위 push, pop 방향이 서로 다르다.<br>
> 
<br>

#### 3) chunk cache<br>
> <br>

> ![image09](https://github.com/user-attachments/assets/78c641fa-ef42-4bfe-850d-5cee568e7fe8)<br>
> <br>
> 청크 캐시의 도입을 처음 생각한 것 자체는 단순히 '무의미한 선형 탐색을 줄이고 싶다' 는 생각에서 출발했다.<br>
> 하지만 설계를 하던 도중 '대량의 선형 탐색이라는 것 자체가 캐시메모리를 갈아엎어 버리는 행위' 임을 깨닳았다. <br>
> 심지어 프로그램이 구동된지 오래되어 청크 내의 메모리가 연속된 페이지에 예쁘게 있는 상황이 아니게된다.<br>
> 즉 파편화 된 상황이라면 페이지를 넘나드는 대량의 캐시라인 무효화는 줄이는것이 항상 맞다는 생각이 들었다. <br>
> <br> 
>

<br>
<br>
<br>

## 4. 성능평가<br>
windows의 기본 동적 할당 기능과 boost, 그리고 직접 만든 라이브러리를 삼자 비교 하였다.<br>
테스트 항목 다음과 같다<br>
TEST 1 : 싱글 스레드 환경에서 메모리를 할당하고, 해제한다. (x축은 kb단위 메모리 크기, y축은 소요 시간)<br>
TEST 2 : 멀티 스레드 환경에서 메모리를 할당하고, 해제한다. (x축은 경합 스레드 갯수, y축은 소요시간, 옵션은 할당 단위)<br>
TEST 3 : 멀티 스레드 환경에서 할당 전담 스레드에서 할당한 메모리를 해제 전담 스레드에서 해제한다. (x축은 스레드 갯수, y축은 소요시간, 옵션은 전담 스레드 종류)<br>
<br>
<br>
<br>
TEST1:<br>
![image10](https://github.com/user-attachments/assets/bc5905ec-83ac-417f-86da-1e7865cace0e)<br>
<br>
1번 테스트의 경우 windows의 LFH영역을 벗어날때 얼마나 오버헤드가 생기는지를 직접 눈으로 확인한다는 의미의 테스트였다.<br>
LFH영역을 넘어서지 않더라도 다소 성능 차이가 발생하기 때문에, boost 또는 본 TLSObjectPool을 사용하는것이 효율적이라 생각 하였으며,<br>
특히 16K 이상의 데이터 (예를 들면 플레이어 캐릭터 데이터, 세션, 인스턴스 정보 등등) 를 다루는 경우 라이브러리 사용이 필수적이라는 결론을 얻었다.<br>
<br>
<br>
TEST2:<br>
![image11](https://github.com/user-attachments/assets/ccc80724-b5c2-434f-8391-3dd22e1fe1f1)<br>
17K 크기의 데이터를 이용한 멀티스레드 환경에서의 경합을 테스트 하였다.<br>
예상대로 OS의 경우 스레드간 경합이 지수적으로 증가함을 알 수 있었다. 하지만 boost의 경우 선형적 증가인지 명확하게 판단이 서지 않아 추가 테스트를 진행하였다.<br>
<br>
![image12](https://github.com/user-attachments/assets/9f55549d-a306-4a90-888a-665b8953d46f)<br>
boost의 경우에는 스레드가 증가함에 따라 아주 약간의 선형적 성능 감소가 있었다고 생각되는 결과였다.<br>
<br>
![image13](https://github.com/user-attachments/assets/35058206-c003-4052-8dff-cf1a87d9fa7e)<br>
추가적으로 LFH가 동작하는 상황에서는 예상외로 boost가 가장 효율이 좋지 않았다.<br>
위에서 추측한 '약간의 선형적 성능 감소' 를 더욱 직접적으로 볼 수 있던 테스트였다.<br>
boost는 내부적으로 스레드간 경합을 조절하는 기능이 분명 존재하지만, 해당 기능의 비용이 스레드 수에 비례한다는 간접적인 근거라고 생각한다.<br>
<br>
<br>
TEST3:<br>
![image14](https://github.com/user-attachments/assets/a815f644-a398-4d46-ad5e-70854fc9397d)<br>
마지막으로 위의 테스트 대비 실제 사용 환경에 유사하다고 생각되는 테스트를 진행하였다.<br>
약간의 노이즈가 껴있지만 어느정도 예측 가능한 결과가 나왔다.<br>
<br>
결국 멀티 스레드 환경에서 개발을 할때, 다량의 메모리 할당 반환이 필요하다면 메모리 관련 라이브러리는 필수적이며,<br>
본 라이브러리는 성능 부분에 한정해서는 boost보다는 더 적합하다는 결론을 내리게 되었다.<br>
