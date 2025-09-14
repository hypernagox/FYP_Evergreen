using UnityEngine;
using System.IO;
using System.Collections.Generic;
using UnityEditor;
using static InstanceJsonExporter;
using static UnityEngine.GraphicsBuffer;

public class InstanceJsonExporter : MonoBehaviour
{
    [SerializeField]
    private string m_jsonDestination = "Assets/InstanceJsonExporter/ExportedInstance.json";
    
    [System.Serializable]
    public struct JsonData
    {
        [System.Serializable]
        public struct InstanceData
        {
            public Vector3 position;
            public Quaternion rotation;
            public Vector3 scale;
        }

        public string prefab;
        public List<InstanceData> instances;
    }

    public void Export()
    {
        Debug.Log("Exporting instance to " + m_jsonDestination);

        Dictionary<GameObject, List<Transform>> instances = new Dictionary<GameObject, List<Transform>>();

        Stack<Transform> stack = new Stack<Transform>();
        stack.Push(transform);

        while (stack.Count > 0)
        {
            Transform current = stack.Pop();

            GameObject prefab = PrefabUtility.GetCorrespondingObjectFromSource(current.gameObject);
            if (prefab == null)
            {
                for (int i = 0; i < current.childCount; i++)
                {
                    stack.Push(current.GetChild(i));
                }
                continue;
            }

            if (!instances.ContainsKey(prefab))
            {
                instances[prefab] = new List<Transform>();
            }
            instances[prefab].Add(current);
        }

        List<JsonData> jsonDatas = new List<JsonData>();

        foreach (KeyValuePair<GameObject, List<Transform>> entry in instances)
        {
            Debug.Log("Exporting " + entry.Value.Count + " instances of " + entry.Key.name);

            JsonData jsonData = new JsonData();
            jsonData.prefab = entry.Key.name;
            jsonData.instances = new List<JsonData.InstanceData>();

            foreach (Transform instance in entry.Value)
            {
                JsonData.InstanceData instanceData = new JsonData.InstanceData();
                instanceData.position = instance.position;
                instanceData.rotation = instance.rotation;
                instanceData.scale = instance.localScale;
                jsonData.instances.Add(instanceData);
            }

            jsonDatas.Add(jsonData);
        }

        string jsonString = "[\n" + string.Join(",\n", System.Array.ConvertAll(jsonDatas.ToArray(), x => JsonUtility.ToJson(x, true))) + "\n]";
        File.WriteAllText(m_jsonDestination, jsonString);
    }
}