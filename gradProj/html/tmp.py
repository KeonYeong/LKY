for j in range(1024):
	curV = (csv_fc1[0][j] - csv_fc1.min()) / (csv_fc1.max() - csv_fc1.min())
	if curV < 0.55 : continue
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
