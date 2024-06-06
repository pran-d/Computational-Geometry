import matplotlib.pyplot as plt
import csv
 
# opening the CSV file
with open('points.csv', mode ='r')as csvfile:
    points = [(float(x), float(y)) for x, y in csv.reader(csvfile, delimiter= ',')]

with open('diagonals.csv', mode='r')as csvfile:
    diagonals = [(float(x), float(y)) for x,y in csv.reader(csvfile, delimiter= ',')]

#combine all x value and y value separately 

#plot polygon without diagonals
plt.figure(1)
for i in range(len(points)):
    if(i==len(points)-1):
        x_values=[points[i][0], points[0][0]]
        y_values=[points[i][1], points[0][1]]
    else:
        x_values=[points[i][0], points[i+1][0]]
        y_values=[points[i][1], points[i+1][1]]
    #now plot x_values and y_values
    plt.plot(x_values, y_values)


#plot polygon with triangulation
plt.figure(2)
for i in range(len(points)):
    if(i==len(points)-1):
        x_values=[points[i][0], points[0][0]]
        y_values=[points[i][1], points[0][1]]
    else:
        x_values=[points[i][0], points[i+1][0]]
        y_values=[points[i][1], points[i+1][1]]
    #now plot x_values and y_values
    plt.plot(x_values, y_values)

for j in range(0,len(diagonals),2):
    x_values=[diagonals[j][0], diagonals[j+1][0]]
    y_values=[diagonals[j][1], diagonals[j+1][1]]
    plt.plot(x_values, y_values)

#setting the scale for x and y axes
plt.xticks([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
plt.yticks([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

plt.show()
