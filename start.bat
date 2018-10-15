docker build --rm -f "dockerfile" -t v-http:latest .
docker run --rm -it -p 3000:3000 v-http:latest