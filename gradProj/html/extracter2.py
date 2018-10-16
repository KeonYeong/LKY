
#-*- coding: utf-8 -*-
import tensorflow as tf
import pandas as pd
from tensorflow.examples.tutorials.mnist import input_data
mnist = input_data.read_data_sets('MNIST_data', one_hot=True)


#interactive로 세션 구현하며, 계산 그래프를 별도로 구성하게끔 한다
sess = tf.InteractiveSession()

#노드 정의
x = tf.placeholder(tf.float32, shape=[None, 784]) #28*28의 이미지라 784,입력텐서
y_ = tf.placeholder(tf.float32, shape=[None, 10]) #출력텐서, 10차원벡터

#이제 CNN

#Weight, Bias를 초기화하는 함수
def weight_variable(shape):
	initial = tf.truncated_normal(shape, stddev = 0.1) #Gradient 0 막으려고, 살짝 잡음
	return tf.Variable(initial)

def bias_variable(shape):
	initial = tf.constant(0.1, shape=shape) #얘는 "죽은 뉴런" 막기 위해 0.1로 초기화
	return tf.Variable(initial)

#conv, pooling
def conv2d(x, W):
	return tf.nn.conv2d(x, W, strides=[1,1,1,1], padding='SAME') #패딩을 입력, 출력 크기 같게 하도록 설정

def max_pool_2x2(x):
	return tf.nn.max_pool(x, ksize=[1,2,2,1], strides=[1,2,2,1], padding='SAME')

#위에선 함수 정의만 했고 이제 진짜로 만듬.
#우선 첫번째 계층
W_conv1 = weight_variable([5, 5, 1, 32]) # 일단 5x5의 32개 필터를 사용할 것, 중간의 1은 입력될 채널 수
b_conv1 = bias_variable([32]) #각 필터에 대해 bias 를 더할 것이라 32

#계층들을 4D로 만들었기 때문에, 입력받을 이미지 역시 4D로 바꿔줘야된다
x_image = tf.reshape(x, [-1,28,28,1]) #여기서 마지막 1은 컬러 채널의 수

#첫번째 계층 적용
h_conv1 = tf.nn.relu(conv2d(x_image, W_conv1) + b_conv1) #곱셈하고 나서 바로 relu를 적용
h_pool1 = max_pool_2x2(h_conv1) #끝나면 맥스풀링, 2x2 이기에 아마 14*14*1*32 일것


#두번째 계층 만듬
W_conv2 = weight_variable([5, 5, 32, 64])
b_conv2 = bias_variable([64])

#두번째 계층 적용
h_conv2 = tf.nn.relu(conv2d(h_pool1, W_conv2) + b_conv2)
h_pool2 = max_pool_2x2(h_conv2) #첫번째랑 하는일은 같다. 풀링을 한번 더한데다가, 64개의 출력을 했으니 크기는 아마 7*7*1*64

#이제 두번째 에서 끝난 값을 1024개의 뉴런으로 연결시키는 계층 (Fully-Connected Layer) 구성
W_fc1 = weight_variable([7*7*64, 1024])
b_fc1 = bias_variable([1024])

#두번째에서 나온 이미지 배열을 일단 2D로 다시 바꿔줘야됨
h_pool2_flat = tf.reshape(h_pool2, [-1, 7*7*64])

#이제 적용
h_fc1 = tf.nn.relu(tf.matmul(h_pool2_flat, W_fc1) + b_fc1)

#오버피팅 방지용 드롭아웃
keep_prob = tf.placeholder(tf.float32) # 이렇게 따로 placeholder를 만들어놓아 확률을 저장하고 이렇게 하면 드롭아웃을 훈련때만 적용되도록 설정 가능
h_fc1_drop = tf.nn.dropout(h_fc1, keep_prob) #실제 드롭아웃

#마지막 소프트맥스 계층 구성
W_fc2 = weight_variable([1024, 10])
b_fc2 = bias_variable([10])

#바로 적용
y_mulConv = tf.matmul(h_fc1_drop, W_fc2) + b_fc2
y_conv = tf.nn.softmax(y_mulConv)

#cost function 정의(cross-entropy)
cross_entropy = tf.reduce_mean(-tf.reduce_sum(y_ * tf.log(y_conv), reduction_indices=[1])) #모든 결과들을 합하는 것, 그리고 그 합들의 평균을 구하는 것

#모델 훈련
train_step = tf.train.AdamOptimizer(1e-4).minimize(cross_entropy) #아까 단일계층과 달리 이번엔 ADAM최적화 알고리즘을 사용함

#모델 평가부분
correct_prediction = tf.equal(tf.argmax(y_conv, 1), tf.argmax(y_, 1)) #argmax는 최댓값을 받아오는데, y(예측)과 y_(실제)의 일치여부를 본다, 리스트로 반환

#따라서 위의 것을 평균을 내고 그 평균을 정확도라 한다. 그러면,
accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

sess.run(tf.global_variables_initializer())

#저장하기 위한 선언
saver = tf.train.Saver()

saver.restore(sess, './myMNIST')


csv_conv1, csv_pool1, csv_conv2, csv_pool2, csv_pool2_flat, csv_fc1, csv_yconv, csv_ymulConv = sess.run([h_conv1, h_pool1, h_conv2, h_pool2, h_pool2_flat, h_fc1_drop, y_conv, y_mulConv], feed_dict={x:mnist.test.images[77:78], keep_prob:1.0})

f = open("./data/csv_pool1_2.csv", 'w')
f.write("weight,img,1st\n")
f.write("0.41,784,6272\n")
f.write("0.41,0,0\n")
for j in range(32):
	for k in range(14):
		for l in range(14):
			curV = (csv_pool1[0][k][l][j] - csv_pool1.min()) / (csv_pool1.max() - csv_pool1.min()) 
			if curV < 0.5 : continue
			tmp = j * 14 * 14 + k * 14 + l
			maxV = -0xFFFF
			for ti in range(2):
				for tj in range(2):
					if csv_conv1[0][k * 2 + ti][l * 2 + tj][j] >= maxV :
						maxV = csv_conv1[0][k * 2 + ti][l * 2 + tj][j]
						tmp2 = k * 28 * 2 + ti * 28 + l * 2 + tj 
			f.write(str(curV) + "," + str(tmp2) + "," + str(tmp) + "\n")

f.close()

f = open("./data/csv_pool2_2.csv", 'w')
f.write("weight,1st,2nd\n")
f.write("0.41,6272,3136\n")
f.write("0.41,0,0\n")
for j in range(64):
	for k in range(7):
		for l in range(7):
			curV = (csv_pool2[0][k][l][j] - csv_pool2.min()) / (csv_pool2.max() - csv_pool2.min()) 
			if curV < 0.5 : continue
			tmp = j * 7 * 7 + k * 7 + l
			maxV = -0xFFFF
			for ti in range(2):
				for tj in range(2):
					if csv_conv2[0][k * 2 + ti][l * 2 + tj][j] >= maxV :
						maxV = csv_conv2[0][k * 2 + ti][l * 2 + tj][j]
						mi = k * 2 + ti
						mj = l * 2 + tj
			maxV = -0xFFFF
			for tf in range(32):
				if csv_pool1[0][mi][mj][tf] >= maxV :
					maxV = csv_pool1[0][mi][mj][tf]
					tmp2 = mi * 14 + mj + tf * 14 * 14
			f.write(str(curV) + "," + str(tmp2) + "," + str(tmp) + "\n")

f.close()

f = open("./data/csv_fc_2.csv", 'w')
f.write("weight,2nd,fc\n")
f.write("0.46,3136,1024\n")
f.write("0.46,0,0\n")
for j in range(1024):
	curV = (csv_fc1[0][j] - csv_fc1.min()) / (csv_fc1.max() - csv_fc1.min())
	if curV < 0.7 : continue
	print(str(j) + " / " + str(1024))
	maxV = 0
	maxI = 0
	for i in range(64):
		tmpV = 0
		for xs in range(7):
			for ys in range(7):
				tmpInd = i * 7 * 7 + xs * 7 + ys
				tmpV += csv_pool2_flat[0][tmpInd] * W_fc1[tmpInd][j]
		fV = tmpV.eval()
		if fV > maxV : 
			maxV = fV
			maxI = i
	for k in range(49):
		f.write(str(curV) + "," + str(maxI * 7 * 7 + k) + "," + str(j) + "\n")
f.close()

f = open("./data/csv_result_2.csv", 'w')
f.write("weight,fc,result\n")
f.write("0.0011,1024,10\n")
f.write("0.0011,0,0\n")
for j in range(10):
	curV = (csv_yconv[0][j] - csv_yconv.min()) / (csv_yconv.max() - csv_yconv.min())
	if curV < 0.8 : continue
	lim = csv_ymulConv[0][j]
	limit = lim / 1024.0
	for k in range(1024) :
		print(str(k) + " / " + str(1024))
		curV2 = csv_fc1[0][k]
		curW = W_fc2[k][j].eval()
		comV = curV2 * curW
		if comV < limit  : continue
		f.write(str(comV) + "," + str(k) + "," + str(j) + "\n")
f.close()
