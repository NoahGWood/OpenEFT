from django.urls import path

from . import views

urlpatterns = [
    path('', views.index, name='index'),
    path('new', views.new, name='new'),
    path('new/step1', views.step1, name='new-step1'),
    path('new/step2', views.step2, name='new-step2'),
    path('new/download', views.download, name='new-download'),
    path('new/resection', views.resection, name='new-resection'),
#    path('new/step3', views.step1, name='new-step3'),
#    path('new/step4', views.step1, name='new-step4'),
#    path('new/step5', views.step1, name='new-step5'),
]