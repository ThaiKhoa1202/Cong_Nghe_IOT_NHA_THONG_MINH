# Sử dụng image Node.js chính thức làm base image
FROM node:14

# Thiết lập thư mục làm việc trong container
WORKDIR /app

# Sao chép file package.json và package-lock.json (nếu có) để cài đặt dependencies
COPY package*.json ./

# Cài đặt dependencies
RUN npm install

# Sao chép mã nguồn của ứng dụng vào thư mục làm việc trong container
COPY . .

# Lệnh để chạy ứng dụng khi container khởi động
CMD ["npm", "start"]
