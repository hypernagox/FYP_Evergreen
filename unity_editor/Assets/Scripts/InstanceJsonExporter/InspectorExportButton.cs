using UnityEditor;
using UnityEngine;

[CustomEditor(typeof(InstanceJsonExporter))]
public class InspectorExportButton : Editor
{
    public override void OnInspectorGUI()
    {
        DrawDefaultInspector();

        InstanceJsonExporter myScript = (InstanceJsonExporter)target;
        if (GUILayout.Button("Export"))
        {
            myScript.Export();
        }
    }
}