import dash
from dash.dependencies import Output, Input, State
# import dash_core_components as dcc
# import dash_html_components as html
from dash.exceptions import PreventUpdate
from dash import html
from dash import dcc
import plotly
import random
import plotly.graph_objs as go
from collections import deque
import numpy as np

import json
from json import JSONEncoder
import connect



class DequeEncoder(JSONEncoder):
    def default(self, obj):
       if isinstance(obj, deque):
          return list(obj)
       return JSONEncoder.default(self, obj)

# varDict = dict()
# varDict["dFront"] = deque(np.zeros(10), maxlen=50)
# X = deque(maxlen=20)
# X.append(1)
# Y = deque(maxlen=20)
# Y.append(1)


################################################################
################################################################
#        APP LAYOUT
################################################################
################################################################

app = dash.Dash(__name__)
app.layout = html.Div(
    [
        dcc.Graph(id='dFront', 
                  animate=True,
                  figure={
                      'layout': {
                          'title':'dFront (front proximitiy sensor)',
                          'xaxis': {'title': 'time steps'},
                          'yaxis': {'title': 'distance [cm]'}
                      }
                  }),
        dcc.Interval(
            id='graph-update',
            interval=1000,
            n_intervals = 0
        ),
        dcc.Store(id="varDict", data=[])
    ]
)


################################################################
################################################################
#    FUNCTIONS
################################################################
################################################################

# This function is triggered to get the newest SERIAL readings
@app.callback(Output('varDict', 'data'),
              Input('graph-update', 'n_intervals'))
def getValues(n_intervals):
    print("-> getValues()")
    varDict = None
    if (n_intervals < 1) or (varDict is None):
        print("First call!")
        varDict = dict()
        for key in communicator.settings["variables"]:
            varDict[key] = [0]

    else:
        print(f"JSON stuff. received data type: {type(varDict)}", flush=True)
        lines = communicator.collectLines(15)
        newVarDict = communicator.parseLines(lines)

        jsonData = json.dumps(newVarDict)
        varDict = communicator.mergeDicts(varDict, newVarDict)

        print("before JSON: ", newVarDict)
        print("jsonData: ", jsonData)

    return {"something": 2345678}

    
# This function updates the graph if the data is updated
@app.callback(Output('dFront', 'figure'),
        [Input('varDict', 'data')])
def updateGraph(varDict):
    print("-> updateGraph()")
    print("just received varDict: ", varDict, ", type: ", type(varDict))
    raise PreventUpdate

    if varDict is None:
        raise PreventUpdate

    X = varDict["dFront"]

    data = plotly.graph_objs.Scatter(
            x=list(X),
            y=list(Y),
            name='Scatter',
            mode= 'lines+markers'
            )

    raise PreventUpdate

    return {'data': data}













"""
@app.callback(Output('dFront', 'figure'),
        [Input('graph-update', 'n_intervals')])

def update_graph_scatter(n):
    # X.append(X[-1]+1)
    # Y.append(Y[-1]+Y[-1]*random.uniform(-0.1,0.1))
    print("Doing something!")
    getVars(n)


    Y = list(varDict['dFront'])
    X = np.arange(-len(Y), 0)
    # print(f"X: {X}")
    # print(f"Y: {Y}")

    data = plotly.graph_objs.Scatter(
            x=list(X),
            y=list(Y),
            name='Scatter',
            mode= 'lines+markers'
            )
    
    # return {'data': [data],
    #         'layout' : go.Layout(xaxis=dict(range=[min(X, default=0),max(X, default=1)]), yaxis=dict(range=[min(Y, default=0)-5,max(Y, default=1)+5]),)}
    return {'data': [data],
    'layout': go.Layout( yaxis=dict(range=[-1,32]))}


def getVars(n):
    lines = communicator.collectLines(25)
    newVarDict = communicator.parseLines(lines)

    print(f"varDict: {varDict['dFront']}")
    print(f"newVarDict: {newVarDict}")

    for key in newVarDict.keys():
        if key in varDict.keys():
            varDict[key].extend(newVarDict[key])
    try:
        if len(newVarDict["dFront"]) == 0:
            print("Did not find any new values.") 
    except KeyError:
        pass
    # print("after merging them")
    # print(f"varDict: {varDict}")



"""



    
################################################################
################################################################
#         M A I N 
################################################################
################################################################



if __name__ == '__main__':
    # create the serial Handler
    communicator = connect.SerialHandler("BT_communication/config.json")
    communicator.connect()

    # Create the varDict
    varDict = dict()
    maxlen = communicator.settings["dash"]["dequeMaxlen"]
    for key in communicator.settings["variables"]:
        print(f"key: {key}")
        varDict[key] = [0,0]

    print(f"varDict: {varDict}")
    print("\n============== Running App ==============")

    app.run_server(debug=False)
