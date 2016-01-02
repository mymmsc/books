VERSION 5.00
Begin VB.Form svrfrm 
   Caption         =   "PipeServer"
   ClientHeight    =   1170
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6030
   LinkTopic       =   "Form1"
   ScaleHeight     =   1170
   ScaleWidth      =   6030
   StartUpPosition =   3  'Windows Default
   Begin VB.Label Label1 
      Caption         =   "Waiting for the client to write to the pipe. If there are no clients connecting, you need to use task manager to kill the server."
      Height          =   615
      Left            =   360
      TabIndex        =   0
      Top             =   240
      Width           =   5175
   End
End
Attribute VB_Name = "svrfrm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

