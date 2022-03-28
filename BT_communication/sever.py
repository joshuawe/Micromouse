import dash
from dash.dependencies import Output, Input
import dash_core_components as dcc
import dash_html_components as html
import plotly
import random
import plotly.graph_objs as go
from collections import deque
import numpy as np

import connect



varDict = dict()
varDict["dFront"] = deque(maxlen=50)
X = deque(maxlen=20)
X.append(1)
Y = deque(maxlen=20)
Y.append(1)


app = dash.Dash(__name__)
app.layout = html.Div(
    [
        dcc.Graph(id='live-graph', animate=True),
        dcc.Interval(
            id='graph-update',
            interval=1000,
            n_intervals = 0
        ),
    ]
)

@app.callback(Output('live-graph', 'figure'),
        [Input('graph-update', 'n_intervals')])

def update_graph_scatter(n):
    # X.append(X[-1]+1)
    # Y.append(Y[-1]+Y[-1]*random.uniform(-0.1,0.1))
    getVars(n)

    Y = varDict['dFront']
    X = np.arange(-len(X), 0)

    data = plotly.graph_objs.Scatter(
            x=list(X),
            y=list(Y),
            name='Scatter',
            mode= 'lines+markers'
            )
    
    return {'data': [data],'layout' : go.Layout(xaxis=dict(range=[min(X, default=0),max(X, default=1)]), yaxis=dict(range=[min(Y, default=0),max(Y, default=1)]),)}

def getVars(n):
    lines = communicator.collectLines(25)
    newVarDict = communicator.parseLines(lines)

    for key in newVarDict.keys():
        if key in varDict.keys():
            varDict[key].extend(newVarDict[key])

    



if __name__ == '__main__':
    # create the serial Handler
    communicator = connect.SerialHandler("BT_communication/config.json")
    communicator.connect()

    # Create the varDict
    varDict = dict()
    maxlen = communicator.settings["dash"]["dequeMaxlen"]
    for key in communicator.settings["variables"]:
        varDict[key] = deque(maxlen=maxlen)

    app.run_server(debug=False)
