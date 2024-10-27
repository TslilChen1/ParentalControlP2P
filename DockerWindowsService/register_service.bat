@echo off
echo Building parentalcontrolchild image...
cd ..\ParentalControlChild
docker build -t parentalcontrolchild .

echo Building parentalcontrolparent image...
cd ..\ParentalControlParent
docker build -t parentalcontrolparent .

echo Running parentalcontrolchild service...
docker run --name parentalcontrolchild --detach parentalcontrolchild

echo Running parentalcontrolparent service...
docker run --name parentalcontrolparent --detach parentalcontrolparent

echo All services registered successfully!
pause
