import cv2
import imutils
import numpy as np


def order_points(pts):
	rect = np.zeros((4, 2), dtype = "float32")
	s = pts.sum(axis = 1)
	rect[0] = pts[np.argmin(s)]
	rect[2] = pts[np.argmax(s)]
	diff = np.diff(pts, axis = 1)
	rect[1] = pts[np.argmin(diff)]
	rect[3] = pts[np.argmax(diff)]
	return rect

def four_point_transform(image, rect):
    (tl, tr, br, bl) = rect
    print(tl, tr, br, bl)
    widthA = np.sqrt(((br[0] - bl[0]) ** 2) + ((br[1] - bl[1]) ** 2))
    widthB = np.sqrt(((tr[0] - tl[0]) ** 2) + ((tr[1] - tl[1]) ** 2))
    maxWidth = max(int(widthA), int(widthB))
    heightA = np.sqrt(((tr[0] - br[0]) ** 2) + ((tr[1] - br[1]) ** 2))
    heightB = np.sqrt(((tl[0] - bl[0]) ** 2) + ((tl[1] - bl[1]) ** 2))
    maxHeight = max(int(heightA), int(heightB))
    dst = np.array([
        [0, 0],
		[maxWidth - 1, 0],
		[maxWidth - 1, maxHeight - 1],
		[0, maxHeight - 1]], dtype = "float32")
    M = cv2.getPerspectiveTransform(rect, dst)
    warped = cv2.warpPerspective(image, M, (maxWidth, maxHeight))
    return warped

def GetEFT(img):
    ratio = img.shape[0]/500.0
    orig = img.copy()
    img = imutils.resize(img, height=500)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # Repeated Closing operation to remove text from the document.
    kernel = np.ones((5, 5), np.uint8)
    img = cv2.morphologyEx(img, cv2.MORPH_CLOSE, kernel, iterations=1)
    gray = cv2.GaussianBlur(img, (5,5),0)
    edged = cv2.Canny(gray, 0, 200)
    cnts = cv2.findContours(edged.copy(), cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    cnts = sorted(cnts, key=cv2.contourArea, reverse=True)[:5]

    for c in cnts:
        peri = cv2.arcLength(c,True)
        approx = cv2.approxPolyDP(c,0.02 * peri, True)
        if(len(approx)==4):
            screenCnt=approx
            break
    cv2.drawContours(img, [screenCnt], -1, (0,255,0),2)
    print("PRIOR POINTS: ")
    print(screenCnt)
    print(type(screenCnt))
    pts = screenCnt.reshape(4,2)*ratio
    print("AFTER POINTS:")
    print(pts)
    print(type(pts))
    rect = order_points(pts)
    print(rect)
    warped = four_point_transform(orig, rect)
    return warped